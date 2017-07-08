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
*   which must be separately licensed for cloused source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/



ToggleButtonList::ToggleButtonList(StringArray& names, Listener* listener_) :
	listener(listener_)
{
	rebuildList(names);
}

void ToggleButtonList::rebuildList(const StringArray &names)
{
	removeAllChildren();
	buttons.clear();

	for (int i = 0; i < names.size(); i++)
	{
		ToggleButton* button = new ToggleButton(names[i]);
		addAndMakeVisible(button);
		button->setColour(ToggleButton::ColourIds::textColourId, Colours::white);
		
		button->setSize(250, 24);
		button->addListener(this);
		buttons.add(button);

	}

	setSize(250, buttons.size() * 26);
	resized();
}

void ToggleButtonList::buttonClicked(Button* b)
{
	const int index = buttons.indexOf(dynamic_cast<ToggleButton*>(b));
	const bool state = b->getToggleState();
	if (listener != nullptr)
	{
		listener->toggleButtonWasClicked(this, index, state);
	}
}

void ToggleButtonList::resized()
{
	int y = 0;

	int width = getWidth();

	for (int i = 0; i < buttons.size(); i++)
	{
		buttons[i]->setBounds(0, y, width, buttons[i]->getHeight());
		y = buttons[i]->getBottom() + 2;
	}
}

void ToggleButtonList::setValue(int index, bool value, NotificationType notify /*= dontSendNotification*/)
{
	if (index >= 0 && index < buttons.size())
	{
		buttons[index]->setToggleState(value, notify);
	}
}

class MidiSourcePanel : public FloatingTileContent,
						public Component,
						public ToggleButtonList::Listener
{
public:

	MidiSourcePanel(FloatingTile* parent) :
		FloatingTileContent(parent)
	{
		StringArray midiInputs = MidiInput::getDevices();

		numMidiDevices = midiInputs.size();

		addAndMakeVisible(midiInputList = new ToggleButtonList(midiInputs, this));

		//midiInputList->setLookAndFeel(&tblaf);
		midiInputList->startTimer(4000);
		AudioProcessorDriver::updateMidiToggleList(getMainController(), midiInputList);
	}

	SET_PANEL_NAME("MidiSources")

	bool showTitleInPresentationMode() const override
	{
		return false;
	}

	void resized() override
	{
		midiInputList->setBounds(getParentShell()->getContentBounds());
	}

	void periodicCheckCallback(ToggleButtonList* list) override
	{
		const StringArray devices = MidiInput::getDevices();

		if (numMidiDevices != devices.size())
		{
			list->rebuildList(devices);
			numMidiDevices = devices.size();
			AudioProcessorDriver::updateMidiToggleList(getMainController(), list);
		}
	}

	void toggleButtonWasClicked(ToggleButtonList* list, int index, bool value) override
	{
		const String midiName = MidiInput::getDevices()[index];

		dynamic_cast<AudioProcessorDriver*>(getMainController())->toggleMidiInput(midiName, value);
	}

private:

	ScopedPointer<ToggleButtonList> midiInputList;

	int numMidiDevices = 0;
};


class MidiChannelPanel : public FloatingTileContent,
						 public Component,
						 public ToggleButtonList::Listener
{
public:

	MidiChannelPanel(FloatingTile* parent) :
		FloatingTileContent(parent)
	{
		StringArray channelNames;
		channelNames.add("All Channels");
		for (int i = 0; i < 16; i++) channelNames.add("Channel " + String(i + 1));

		addAndMakeVisible(channelList = new ToggleButtonList(channelNames, this));

		//channelList->setLookAndFeel(&tblaf);

		HiseEvent::ChannelFilterData* channelFilterData = getMainController()->getMainSynthChain()->getActiveChannelData();

		channelList->setValue(0, channelFilterData->areAllChannelsEnabled());
		for (int i = 0; i < 16; i++)
		{
			channelList->setValue(i + 1, channelFilterData->isChannelEnabled(i));
		}

	}

	SET_PANEL_NAME("MidiChannelList");

	void toggleButtonWasClicked(ToggleButtonList* list, int index, bool value) override
	{
		HiseEvent::ChannelFilterData *newData = getMainController()->getMainSynthChain()->getActiveChannelData();

		if (index == 0)
		{
			newData->setEnableAllChannels(value);
		}
		else
		{
			newData->setEnableMidiChannel(index - 1, value);
		}
	}

	void periodicCheckCallback(ToggleButtonList* list) override
	{
	}

	void resized() override
	{
		channelList->setBounds(getParentShell()->getContentBounds());
	}

private:

	ScopedPointer<ToggleButtonList> channelList;

};

class TuningWindow::Panel : public FloatingTileContent,
	public Component
{
public:

	Panel(FloatingTile* parent) :
		FloatingTileContent(parent)
	{
		addAndMakeVisible(window = new TuningWindow(getMainController()));
	}

	SET_PANEL_NAME("TuningWindow");

	void resized()
	{
		window->setBounds(getLocalBounds());
	};

	bool showTitleInPresentationMode() const override { return false; }

private:

	ScopedPointer<TuningWindow> window;

};

#define ADD(x) propIds.add(Identifier(#x));

CustomSettingsWindow::CustomSettingsWindow(MainController* mc_) :
	mc(mc_)
{
	ADD(Driver);
	ADD(Device);
	ADD(Output);
	ADD(BufferSize);
	ADD(SampleRate);
	ADD(ScaleFactor);
	ADD(StreamingMode);
	ADD(SustainCC);
	ADD(ClearMidiCC);
	ADD(SampleLocation);
	ADD(DebugMode);
    
    for(int i = 0; i < (int)Properties::numProperties; i++)
    {
        properties[i] = true;
    }
    
    addAndMakeVisible(deviceSelector = new ComboBox("Driver"));
    addAndMakeVisible(soundCardSelector = new ComboBox("Device"));
    addAndMakeVisible(outputSelector = new ComboBox("Output"));
    addAndMakeVisible(sampleRateSelector = new ComboBox("Sample Rate"));
    addAndMakeVisible(bufferSelector = new ComboBox("Buffer Sizes"));
    addAndMakeVisible(sampleRateSelector = new ComboBox("Sample Rate"));
        
    deviceSelector->addListener(this);
    soundCardSelector->addListener(this);
    outputSelector->addListener(this);
    bufferSelector->addListener(this);
    sampleRateSelector->addListener(this);
        
    deviceSelector->setLookAndFeel(&plaf);
    soundCardSelector->setLookAndFeel(&plaf);
    outputSelector->setLookAndFeel(&plaf);
    bufferSelector->setLookAndFeel(&plaf);
    sampleRateSelector->setLookAndFeel(&plaf);
    
	addAndMakeVisible(ccSustainSelector = new ComboBox("Sustain CC"));
    addAndMakeVisible(scaleFactorSelector = new ComboBox("Scale Factor"));
	addAndMakeVisible(diskModeSelector = new ComboBox("Hard Disk"));
	addAndMakeVisible(clearMidiLearn = new TextButton("Clear MIDI CC"));
	addAndMakeVisible(relocateButton = new TextButton("Change sample folder location"));
	addAndMakeVisible(debugButton = new TextButton("Toggle Debug Mode"));

	ccSustainSelector->addListener(this);
	scaleFactorSelector->addListener(this);
	diskModeSelector->addListener(this);
	clearMidiLearn->addListener(this);
	relocateButton->addListener(this);
	debugButton->addListener(this);

	ccSustainSelector->setLookAndFeel(&plaf);
	scaleFactorSelector->setLookAndFeel(&plaf);
	diskModeSelector->setLookAndFeel(&plaf);
	clearMidiLearn->setLookAndFeel(&blaf);
	debugButton->setLookAndFeel(&blaf);
	clearMidiLearn->setColour(TextButton::ColourIds::textColourOffId, Colours::white);
	clearMidiLearn->setColour(TextButton::ColourIds::textColourOnId, Colours::white);
	relocateButton->setLookAndFeel(&blaf);
	relocateButton->setColour(TextButton::ColourIds::textColourOffId, Colours::white);
	relocateButton->setColour(TextButton::ColourIds::textColourOnId, Colours::white);
	debugButton->setColour(TextButton::ColourIds::textColourOffId, Colours::white);
	debugButton->setColour(TextButton::ColourIds::textColourOnId, Colours::white);

	rebuildMenus(true, true);

#if HISE_IOS && HISE_IPHONE
    setSize(250, 180);
#else

#if IS_STANDALONE_FRONTEND
	setSize(320, 470);
#else
	setSize(320, 260);
#endif

#endif
}

CustomSettingsWindow::~CustomSettingsWindow()
{
	deviceSelector->removeListener(this);
	sampleRateSelector->removeListener(this);
	bufferSelector->removeListener(this);
	soundCardSelector->removeListener(this);
	outputSelector->removeListener(this);
    
	clearMidiLearn->removeListener(this);
	relocateButton->removeListener(this);
	diskModeSelector->removeListener(this);
	scaleFactorSelector->removeListener(this);
	debugButton->removeListener(this);

	deviceSelector = nullptr;
	bufferSelector = nullptr;
	sampleRateSelector = nullptr;
	diskModeSelector = nullptr;
	clearMidiLearn = nullptr;
	relocateButton = nullptr;
	debugButton = nullptr;
}

void CustomSettingsWindow::rebuildMenus(bool rebuildDeviceTypes, bool rebuildDevices)
{
	AudioProcessorDriver* driver = dynamic_cast<AudioProcessorDriver*>(mc);
    
    if(HiseDeviceSimulator::isStandalone())
    {
        const OwnedArray<AudioIODeviceType> *devices = &driver->deviceManager->getAvailableDeviceTypes();
        
        bufferSelector->clear();
        sampleRateSelector->clear();
        outputSelector->clear();
        
        
        if (rebuildDeviceTypes)
        {
            deviceSelector->clear();
            
            for (int i = 0; i < devices->size(); i++)
            {
                deviceSelector->addItem(devices->getUnchecked(i)->getTypeName(), i + 1);
            };
        }
        
        AudioIODevice* currentDevice = driver->deviceManager->getCurrentAudioDevice();
        
        if (currentDevice != nullptr)
        {
            const int thisDevice = devices->indexOf(driver->deviceManager->getCurrentDeviceTypeObject());
            
            AudioIODeviceType *currentDeviceType = devices->getUnchecked(thisDevice);
            
            if (rebuildDeviceTypes && thisDevice != -1)
            {
                deviceSelector->setSelectedItemIndex(thisDevice, dontSendNotification);
            }
            
            Array<int> bufferSizes = currentDevice->getAvailableBufferSizes();
            
            
            
            if (bufferSizes.size() > 7)
            {
                Array<int> powerOfTwoBufferSizes;
                powerOfTwoBufferSizes.ensureStorageAllocated(6);
                if (bufferSizes.contains(64)) powerOfTwoBufferSizes.add(64);
                if (bufferSizes.contains(128)) powerOfTwoBufferSizes.add(128);
                if (bufferSizes.contains(256)) powerOfTwoBufferSizes.add(256);
                if (bufferSizes.contains(512)) powerOfTwoBufferSizes.add(512);
                if (bufferSizes.contains(1024)) powerOfTwoBufferSizes.add(1024);
                
                if (powerOfTwoBufferSizes.size() > 2)
                    bufferSizes.swapWith(powerOfTwoBufferSizes);
            }
            
            int defaultBufferSize = currentDevice->getDefaultBufferSize();
            
            bufferSizes.addIfNotAlreadyThere(defaultBufferSize);
            
            bufferSizes.sort();
            
            outputSelector->addItemList(getChannelPairs(currentDevice), 1);
            const int thisOutputName = (currentDevice->getActiveOutputChannels().getHighestBit() - 1) / 2;
            outputSelector->setSelectedItemIndex(thisOutputName, dontSendNotification);
            
            if (rebuildDevices)
            {
                soundCardSelector->clear();
                
                StringArray soundCardNames = currentDeviceType->getDeviceNames(false);
                soundCardSelector->addItemList(soundCardNames, 1);
                const int soundcardIndex = soundCardNames.indexOf(currentDevice->getName());
                soundCardSelector->setSelectedItemIndex(soundcardIndex, dontSendNotification);
            }
            
            
            for (int i = 0; i < bufferSizes.size(); i++)
            {
                bufferSelector->addItem(String(bufferSizes[i]) + String(" Samples"), i + 1);
            }
            
            const int thisBufferSize = currentDevice->getCurrentBufferSizeSamples();
            
            bufferSelector->setSelectedItemIndex(bufferSizes.indexOf(thisBufferSize), dontSendNotification);
            

            Array<double> allSamplerates = currentDevice->getAvailableSampleRates();
            Array<double> samplerates;
            
            if (allSamplerates.contains(44100.0)) samplerates.add(44100.0);
            if (allSamplerates.contains(48000.0)) samplerates.add(48000.0);
            if (allSamplerates.contains(88200.0)) samplerates.add(88200.0);
            if (allSamplerates.contains(96000.0)) samplerates.add(96000.0);
            if (allSamplerates.contains(176400.0)) samplerates.add(176400.0);
            if (allSamplerates.contains(192000.0)) samplerates.add(192000.0);

			for (int i = 0; i < samplerates.size(); i++)
            {
                sampleRateSelector->addItem(String(samplerates[i], 0), i + 1);
            }
            
            const double thisSampleRate = currentDevice->getCurrentSampleRate();
            
            sampleRateSelector->setSelectedItemIndex(samplerates.indexOf(thisSampleRate), dontSendNotification);
        }
        else
        {
            String message = "The Audio driver " + deviceSelector->getText() + " could not be opened.";
            
            PresetHandler::showMessageWindow("Audio Driver Initialisation Error", message, PresetHandler::IconType::Error);
            
            driver->deviceManager->initialiseWithDefaultDevices(0, 2);
            
            if(!loopProtection)
            {
                loopProtection=true;
                rebuildMenus(true, true);
            }
        }
    }
    
    scaleFactorSelector->clear();
    scaleFactorSelector->addItem("100%", 1);
    scaleFactorSelector->addItem("75%", 2);
    
	diskModeSelector->clear();
	diskModeSelector->addItem("Fast - SSD", 1);
	diskModeSelector->addItem("Slow - HDD", 2);

	ccSustainSelector->clear(dontSendNotification);

	for (int i = 1; i < 127; i++)
	{
		ccSustainSelector->addItem(String("CC ") + String(i), i);
	}

	ccSustainSelector->setSelectedId(driver->ccSustainValue, dontSendNotification);
	diskModeSelector->setSelectedItemIndex(driver->diskMode, dontSendNotification);
	scaleFactorSelector->setSelectedItemIndex(driver->scaleFactor == 1.0 ? 0 : 1, dontSendNotification);
}

void CustomSettingsWindow::buttonClicked(Button* b)
{
	if (b == relocateButton)
	{
#if USE_FRONTEND

		File oldLocation = ProjectHandler::Frontend::getSampleLocationForCompiledPlugin();

		FileChooser fc("Select new Sample folder", oldLocation);

		if (fc.browseForDirectory())
		{
			File newLocation = fc.getResult();

			if (newLocation.isDirectory())
			{
				ProjectHandler::Frontend::setSampleLocation(newLocation);

				auto fp = dynamic_cast<FrontendSampleManager*>(mc);
				
				fp->checkAllSampleReferences();
				
				if (fp->areSampleReferencesCorrect())
				{
					PresetHandler::showMessageWindow("Sample Folder relocated", "You need to reload the plugin to complete this step", PresetHandler::IconType::Info);
				}
			}
		}

#else
		PresetHandler::showMessageWindow("Only useful in compiled plugin", "This button only works for the compiled plugin");
#endif
	}
	else if (b == clearMidiLearn)
	{
		ScopedLock sl(mc->getLock());

		mc->getMacroManager().getMidiControlAutomationHandler()->clear();
	}
	else if (b == debugButton)
	{
		mc->getDebugLogger().toggleLogging();
	}
}

void CustomSettingsWindow::flipEnablement(AudioDeviceManager* manager, const int row)
{
	AudioDeviceManager::AudioDeviceSetup config;
	manager->getAudioDeviceSetup(config);

	BigInteger& original = config.outputChannels;

	original.clear();
	original.setBit(row * 2, 1);
	original.setBit(row * 2 + 1, 1);

	config.useDefaultOutputChannels = false;

	manager->setAudioDeviceSetup(config, true);
}

String CustomSettingsWindow::getNameForChannelPair(const String& name1, const String& name2)
{
	String commonBit;

	for (int j = 0; j < name1.length(); ++j)
		if (name1.substring(0, j).equalsIgnoreCase(name2.substring(0, j)))
			commonBit = name1.substring(0, j);

	// Make sure we only split the name at a space, because otherwise, things
	// like "input 11" + "input 12" would become "input 11 + 2"
	while (commonBit.isNotEmpty() && !CharacterFunctions::isWhitespace(commonBit.getLastCharacter()))
		commonBit = commonBit.dropLastCharacters(1);

	return name1.trim() + " + " + name2.substring(commonBit.length()).trim();
}

StringArray CustomSettingsWindow::getChannelPairs(AudioIODevice* currentDevice)
{
	if (currentDevice != nullptr)
	{
		StringArray items = currentDevice->getOutputChannelNames();

		StringArray pairs;

		for (int i = 0; i < items.size(); i += 2)
		{
			const String& name = items[i];

			if (i + 1 >= items.size())
				pairs.add(name.trim());
			else
				pairs.add(getNameForChannelPair(name, items[i + 1]));
		}

		return pairs;
	}

	return StringArray();
}

void CustomSettingsWindow::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	AudioProcessorDriver* driver = dynamic_cast<AudioProcessorDriver*>(mc);

	

	if (comboBoxThatHasChanged == deviceSelector)
	{
		const String deviceName = deviceSelector->getText();

		driver->setAudioDeviceType(deviceName);

		rebuildMenus(false, true);
	}
	else if (comboBoxThatHasChanged == soundCardSelector)
	{
		const String name = soundCardSelector->getText();

		driver->setAudioDevice(name);

		rebuildMenus(false, false);

		DBG(name);
	}
	else if (comboBoxThatHasChanged == outputSelector)
	{
		const String outputName = outputSelector->getText();

		flipEnablement(driver->deviceManager, outputSelector->getSelectedItemIndex());

		//driver->setOutputChannelName(outputSelector->getSelectedItemIndex());

		//DBG(outputName);
	}
	else if (comboBoxThatHasChanged == bufferSelector)
	{
		const int bufferSize = bufferSelector->getText().getIntValue();
		driver->setCurrentBlockSize(bufferSize);
	}
	else if (comboBoxThatHasChanged == sampleRateSelector)
	{
		const double sampleRate = (double)sampleRateSelector->getText().getIntValue();
		driver->setCurrentSampleRate(sampleRate);
	}
	else if (comboBoxThatHasChanged == ccSustainSelector)
	{
		int newSustainCC = comboBoxThatHasChanged->getSelectedId();

		driver->ccSustainValue = newSustainCC;

		mc->getEventHandler().addCCRemap(newSustainCC, 64);
	}
	else if (comboBoxThatHasChanged == scaleFactorSelector)
	{
		double scaleFactor = (scaleFactorSelector->getSelectedItemIndex() == 0) ? 1.0 : 0.85;

		driver->setGlobalScaleFactor(scaleFactor);

#if USE_FRONTEND

		auto fpe = findParentComponentOfClass<FrontendProcessorEditor>();

		if (fpe != nullptr)
		{
			fpe->setGlobalScaleFactor((float)scaleFactor);
		}
#endif


	}
	else if (comboBoxThatHasChanged == diskModeSelector)
	{
		const int index = diskModeSelector->getSelectedItemIndex();

		driver->diskMode = index;

		mc->getSampleManager().setDiskMode((MainController::SampleManager::DiskMode)index);
	}
}

void CustomSettingsWindow::paint(Graphics& g)
{
	g.setColour(Colours::white);

	g.setFont(GLOBAL_BOLD_FONT());

	int y = 0;

	if (isOn(Properties::Driver))
	{
		g.drawText("Driver", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::Device))
	{
		g.drawText("Device", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::Output))
	{
		g.drawText("Output", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::BufferSize))
	{
		g.drawText("Buffer Size", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::SampleRate))
	{
		g.drawText("Sample Rate", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::ScaleFactor))
	{
		g.drawText("Scale Factor", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::StreamingMode))
	{
		g.drawText("Streaming Mode", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::SustainCC))
	{
		g.drawText("Sustain CC", 0, y, getWidth() / 2 - 30, 30, Justification::centredRight);
		y += 40;
	}
	if (isOn(Properties::ClearMidiCC))
	{
		y += 40;
	}
	if (isOn(Properties::SampleLocation))
	{

#if USE_BACKEND
		const String samplePath = GET_PROJECT_HANDLER(mc->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::Samples).getFullPathName();
#else
		const String samplePath = ProjectHandler::Frontend::getSampleLocationForCompiledPlugin().getFullPathName();
#endif

		g.setFont(GLOBAL_BOLD_FONT());
		g.drawText("Sample Location:", 15, y, getWidth() - 30, 30, Justification::centredTop);

		g.setFont(GLOBAL_FONT());
		g.drawText(samplePath, 10, y, getWidth() - 20, 30, Justification::centredBottom);

		y += 80;
	}
}

void CustomSettingsWindow::resized()
{
    int y = 0;
    
	if (isOn(Properties::Device))
	{
		deviceSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else deviceSelector->setVisible(false);
	
	if (isOn(Properties::Driver))
	{
		soundCardSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else soundCardSelector->setVisible(false);

	if (isOn(Properties::Output))
	{
		outputSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else outputSelector->setVisible(false);

	if (isOn(Properties::BufferSize))
	{
		bufferSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else bufferSelector->setVisible(false);

	if (isOn(Properties::SampleRate))
	{
		sampleRateSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else sampleRateSelector->setVisible(false);

	if (isOn(Properties::ScaleFactor))
	{
		scaleFactorSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else scaleFactorSelector->setVisible(false);

	if (isOn(Properties::StreamingMode))
	{
		diskModeSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else diskModeSelector->setVisible(false);

	if (isOn(Properties::SustainCC))
	{
		ccSustainSelector->setBounds(getWidth() / 2 - 20, y, getWidth() / 2, 30);
		y += 40;
	}
	else ccSustainSelector->setVisible(false);

	if (isOn(Properties::ClearMidiCC))
	{
		clearMidiLearn->setBounds(10, y, getWidth() - 20, 30);
		y += 40;
	}
	else clearMidiLearn->setVisible(false);

	if (isOn(Properties::SampleLocation))
	{
		y += 40;
		relocateButton->setBounds(10, y, getWidth() - 20, 30);
		y += 40;
	}
	else relocateButton->setVisible(false);

	if (isOn(Properties::DebugMode))
	{
		debugButton->setBounds(10, y, getWidth() - 20, 30);
	}
	else debugButton->setVisible(false);
}

class CustomSettingsWindow::Panel: public FloatingTileContent,
public Component
{
public:

#define SET(x) storePropertyInObject(obj, (int)x, var(window->properties[(int)x]));
	
	var toDynamicObject() const override
	{
		var obj = FloatingTileContent::toDynamicObject();

		SET(Properties::Driver);
		SET(Properties::Device);
		SET(Properties::Output);
		SET(Properties::BufferSize);
		SET(Properties::SampleRate);
		SET(Properties::StreamingMode);
		SET(Properties::ScaleFactor);
		SET(Properties::SustainCC);
		SET(Properties::ClearMidiCC);
		SET(Properties::SampleLocation);
		SET(Properties::DebugMode);
		
		return obj;
	}

#undef SET

#define SET(x) window->setProperty(x, getPropertyWithDefault(object, (int)x));

	void fromDynamicObject(const var& object) override
	{
		FloatingTileContent::fromDynamicObject(object);

		SET(Properties::Driver);
		SET(Properties::Device);
		SET(Properties::Output);
		SET(Properties::BufferSize);
		SET(Properties::SampleRate);
		SET(Properties::StreamingMode);
		SET(Properties::ScaleFactor);
		SET(Properties::SustainCC);
		SET(Properties::ClearMidiCC);
		SET(Properties::SampleLocation);
		SET(Properties::DebugMode);
		
	}

#undef SET

#define SET(x) RETURN_DEFAULT_PROPERTY_ID(index, (int)x, window->propIds[(int)x - (int)FloatingTileContent::numPropertyIds]);

	Identifier getDefaultablePropertyId(int index) const override
	{
		if (index < FloatingTileContent::numPropertyIds)
			return FloatingTileContent::getDefaultablePropertyId(index);

		SET(Properties::Driver);
		SET(Properties::Device);
		SET(Properties::Output);
		SET(Properties::BufferSize);
		SET(Properties::SampleRate);
		SET(Properties::StreamingMode);
		SET(Properties::ScaleFactor);
		SET(Properties::SustainCC);
		SET(Properties::ClearMidiCC);
		SET(Properties::SampleLocation);
		SET(Properties::DebugMode);

		jassertfalse;
		return{};
	}

#undef SET

#define SET(x) RETURN_DEFAULT_PROPERTY(index, (int)x, true);

	var getDefaultProperty(int index) const override
	{
		if (index < FloatingTileContent::numPropertyIds)
			return FloatingTileContent::getDefaultProperty(index);

		SET(Properties::Driver);
		SET(Properties::Device);
		SET(Properties::Output);
		SET(Properties::BufferSize);
		SET(Properties::SampleRate);
		SET(Properties::StreamingMode);
		SET(Properties::ScaleFactor);
		SET(Properties::SustainCC);
		SET(Properties::ClearMidiCC);
		SET(Properties::SampleLocation);
		SET(Properties::DebugMode);

		jassertfalse;
		return{};
	}

	
	Panel(FloatingTile* parent) :
		FloatingTileContent(parent)
	{
		addAndMakeVisible(window = new CustomSettingsWindow(getMainController()));
	}

	SET_PANEL_NAME("CustomSettings");

	void resized()
	{
		window->setBounds(getLocalBounds());
	};

	bool showTitleInPresentationMode() const override { return false; }

private:

	ScopedPointer<CustomSettingsWindow> window;

};

CombinedSettingsWindow::CombinedSettingsWindow(MainController* mc_):
	mc(mc_)
{
	setLookAndFeel(&klaf);

	addAndMakeVisible(closeButton = new ShapeButton("Close", Colours::white.withAlpha(0.6f), Colours::white, Colours::white));

	Path closeShape;

	closeShape.loadPathFromData(HiBinaryData::ProcessorEditorHeaderIcons::closeIcon, sizeof(HiBinaryData::ProcessorEditorHeaderIcons::closeIcon));
	closeButton->setShape(closeShape, true, true, true);
	closeButton->addListener(this);
	addAndMakeVisible(settings = new CustomSettingsWindow(mc));

	StringArray midiNames = MidiInput::getDevices();
	numMidiDevices = midiNames.size();

	addAndMakeVisible(midiSources = new ToggleButtonList(midiNames, this));
	midiSources->startTimer(4000);

	settings->setLookAndFeel(&klaf);

	AudioProcessorDriver::updateMidiToggleList(mc, midiSources);

#if HISE_IOS
	setSize(400, settings->getHeight() + midiSources->getHeight() + 70);
#else
    setSize(600, settings->getHeight() + midiSources->getHeight() + 70);
#endif

	closeButton->setTooltip("Close this dialog");


}

CombinedSettingsWindow::~CombinedSettingsWindow()
{
	closeButton = nullptr;
	settings = nullptr;
	mc = nullptr;
	midiSources = nullptr;
}

void CombinedSettingsWindow::resized()
{
	closeButton->setBounds(getWidth() - 24, 2, 20, 20);
	settings->setTopLeftPosition((getWidth() - settings->getWidth()) / 2, 40);
	midiSources->setBounds((getWidth() - settings->getWidth()) / 2, settings->getBottom() + 15, settings->getWidth(), midiSources->getHeight());
}

void CombinedSettingsWindow::paint(Graphics &g)
{
	g.fillAll(Colour(0xff111111).withAlpha(0.92f));

	g.setColour(Colours::white.withAlpha(0.2f));
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white.withAlpha(0.1f));

	Rectangle<int> title1(1, 1, getWidth() - 2, 25);
	Rectangle<int> title2(1, settings->getBottom() - 22, getWidth() - 2, 25);

	g.fillRect(title1);
	g.fillRect(title2);

	g.setColour(Colours::white);
	g.setFont(GLOBAL_BOLD_FONT());

	g.drawText("Audio Settings", title1, Justification::centred);
	g.drawText("MIDI Inputs", title2, Justification::centred);
}

void CombinedSettingsWindow::toggleButtonWasClicked(ToggleButtonList* /*list*/, int index, bool value)
{
	const String midiName = MidiInput::getDevices()[index];

	dynamic_cast<AudioProcessorDriver*>(mc)->toggleMidiInput(midiName, value);
}

void CombinedSettingsWindow::buttonClicked(Button* )
{
    
    dynamic_cast<AudioProcessorDriver*>(mc)->saveDeviceSettingsAsXml();
    ScopedPointer<XmlElement> deviceData = dynamic_cast<AudioProcessorDriver*>(mc)->deviceManager->createStateXml();
    dynamic_cast<AudioProcessorDriver*>(mc)->initialiseAudioDriver(deviceData);

    
	findParentComponentOfClass<ModalBaseWindow>()->clearModalComponent();
}

void CombinedSettingsWindow::periodicCheckCallback(ToggleButtonList* list)
{
	const StringArray devices = MidiInput::getDevices();

	if (numMidiDevices != devices.size())
	{
		list->rebuildList(devices);
		numMidiDevices = devices.size();

		AudioProcessorDriver::updateMidiToggleList(mc, midiSources);
	}
}
