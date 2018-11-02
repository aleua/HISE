/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#pragma once

namespace hise
{

/** @brief A gateway drug to the full HISE world.

	This namespace acts as higher level API to the HISE code base and provides helper classes and convenient functions to tuck away the complexity of the full code base. The goal of this namespace is to provide access to the HISE codebase for the majority of use cases
	that are required by people using HISE for their projects. The full API class list of all HISE classes can be overwhelming, so this is supposed to be a starting point for people diving into the C++ side of HISE.

	Also, all classes in this API are guaranteed to meet stricter API standards regarding documentation and backwards compatibility.

	In this namespace you get:

	- a convenient place to look for magic numbers: the IDs namespace
	- a class that helps building / changing the module architecture of HISE: the Builder
	- a wrapper around a module that gets notified when a parameter changes: the Reference
	- a class that loads embedded data: the Pool
	- a class that handles the (asynchronous) and lockfree execution of larger tasks: the TaskAfterSuspension.


*/
namespace raw
{

using namespace juce;

struct AttributeItem
{
	int index;
	float value;
};

/** a collection of attribute key/value pairs. */
using AttributeCollection = std::vector<AttributeItem>;

/** The builder is a low overhead helper class that provides functions to add modules.

Create one of those, supply the main controller instance and call its methods to build up the architecture of your plugin. 

\code
// The main controller of this project
auto mc = getMainController();

// The root container of this project
auto masterContainer = mc->getMainSynthChain();

Builder b(mc);

// Adds a sine wave generator
auto sine = b.create<hise::SineSynth>(masterContainer, IDs::Chains::Direct);
sine->setAttribute(hise::SineSynth::OctaveTranspose, 5.0f, sendNotification);

// Adds a reverb to the sine wave generator
auto reverb = b.create<hise::SimpleReverbEffect>(sine, IDs::Chains::FX);
reverb->setAttribute(hise::SimpleReverbEffect::WetLevel, 0.5f, sendNotification);
\endcode

Important: This class can only be used from the IDs::Threads::Loading thread in a suspended state.
If you want to use this class from another thread, wrap it into a lambda and give it to the TaskAfterSuspension class.

*/
class Builder
{
public:

	

	Builder(MainController* mc_) :
		mc(mc_)
	{
		// if you're hitting this assertion, it means that you haven't killed the voices properly.
		// Use an raw::TaskAfterSuspension object for this.
		jassert(LockHelpers::freeToGo(mc));

		
	};

	/** Finds and creates a reference object to the module with the given ID. */
	template <class T> Reference<T> find(const String& name);

	/** Adds the given module to the parent processor. Specify the chainIndex for modulators / effects. */
	template <class T> void add(T* processor, Processor* parent, int chainIndex = IDs::Chains::Direct);

	/** Creates a module of the given class and adds it to the parent with the specified
	chainIndex. See ChainIndexes.

	This only works with HISE modules that are registered at one of the factories, so if you want to add a custom module, use the add() function instead.
	*/
	template <class T> T* create(Processor* parent, int chainIndex = IDs::Chains::Direct);

	/** Removes a processor and all its child processors from the signal path. */
	template <class T> bool remove(Processor* p);

	/** Creates a module from the given Base64 encoded String and adds it to the parent module with the suppliedChainIndex. */
	Processor* createFromBase64State(const String& base64EncodedString, Processor* parent, int chainIndex = IDs::Chains::Direct);

	/** Sets all the attributes from the given collection.

	You can use std::initialiser_lists for a clean syntax:

	\code{.cpp}
	AttributeCollection c =
	{
		{ SimpleEnvelope::Attack, 10.0f },
		{ SimpleEnvelope::Release, 248.0f }
	};

	for(auto* envelope: myEnvelopes)
		builder.setAttributes(envelope, c);
	\endcode

	*/
	void setAttributes(Processor* p, const AttributeCollection& collection);

private:

	template <class T> T* addInternal(Processor* p, Chain* c);

	MainController * mc;
};


} // namespace raw

}