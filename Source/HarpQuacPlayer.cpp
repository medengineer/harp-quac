/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "HarpQuacPlayer.h"
#include "AnalogPatternGenerator.h"

HarpQuacPlayerBackground::HarpQuacPlayerBackground()
{
    currentWaveform = Path();
}

void HarpQuacPlayerBackground::updateCurrentWaveform (Pattern* pattern)
{
    if (pattern == nullptr)
    {
        currentWaveform.clear();
        repaint();
        return;
    }

    updateCurrentWaveform (pattern->samples, pattern->maxVoltage);
}

void HarpQuacPlayerBackground::updateCurrentWaveform (const Array<float>& samples, float maxVoltage)
{
    currentWaveform.clear();

    if (samples.isEmpty())
    {
        repaint();
        return;
    }

    float numSamples = (float) samples.size();

    currentWaveform.startNewSubPath (0, 1);

    float lastSample = samples[0];

    if (maxVoltage == 0.0f)
        maxVoltage = 5.0f;

    currentWaveform.lineTo (0, 1 - lastSample / maxVoltage);

    for (int i = 0; i < samples.size(); i++)
    {
        if (samples[i] != lastSample)
        {
            currentWaveform.lineTo (float (i) / numSamples, 1 - lastSample / maxVoltage);
            currentWaveform.lineTo (float (i) / numSamples, 1 - samples[i] / maxVoltage);
            lastSample = samples[i];
        }
    }

    pathTransform = currentWaveform.getTransformToScaleToFit (140, 30, 160, 70, false);

    repaint();
}

void HarpQuacPlayerBackground::paint (Graphics& g)
{
    g.setColour (findColour (ThemeColours::defaultText));
    g.drawRect (0, 0, getWidth(), getHeight(), 1.0);

    g.setColour (Colours::orange);
    g.strokePath (currentWaveform, PathStrokeType (1.0), pathTransform);
}

HarpQuacPlayer::HarpQuacPlayer()
    : nextPatternId (1)
{
    background = std::make_unique<HarpQuacPlayerBackground>();
    background->setBounds (0, 0, 320, 105);
    addAndMakeVisible (background.get());

    modeSelector = std::make_unique<ComboBox>();
    modeSelector->setBounds (12, 8, 120, 20);
    modeSelector->addListener (this);
    modeSelector->addItem ("GENERATE", (int) WaveOutputMode::GENERATE);
    modeSelector->addItem ("LISTEN", (int) WaveOutputMode::LISTEN);
    modeSelector->setSelectedId ((int) WaveOutputMode::GENERATE, dontSendNotification);
    addAndMakeVisible (modeSelector.get());

    patternSelector = std::make_unique<ComboBox>();
    patternSelector->setBounds (12, 32, 120, 20);
    patternSelector->addListener (this);
    patternSelector->setEditableText (true);
    addAndMakeVisible (patternSelector.get());

    triggerSelector = std::make_unique<ComboBox>();
    triggerSelector->setBounds (12, 100, 120, 20);
    triggerSelector->addListener (this);
    triggerSelector->setEnabled (false);
    triggerSelector->addItem ("NONE", 1);
    triggerSelector->setSelectedId (1);

    outputChannelSelector = std::make_unique<ComboBox>();
    outputChannelSelector->setBounds (12, 56, 120, 20);
    outputChannelSelector->addListener (this);
    for (int i = 0; i < kNumAoChannels; i++)
        outputChannelSelector->addItem ("Output: AO " + String (i), i + 1);
    outputChannelSelector->setSelectedId (1, dontSendNotification);
    addAndMakeVisible (outputChannelSelector.get());

    enableButton = std::make_unique<UtilityButton> ("DISABLED");
    enableButton->setBounds (12, 80, 70, 20);
    enableButton->addListener (this);
    addAndMakeVisible (enableButton.get());

    startStopButton = std::make_unique<UtilityButton> ("RUN");
    startStopButton->setBounds (90, 80, 42, 20);
    startStopButton->addListener (this);
    startStopButton->setEnabled (false);
    addAndMakeVisible (startStopButton.get());

    pulsePatternButton = std::make_unique<UtilityButton> ("Pulse");
    pulsePatternButton->setCorners (true, false, true, false);
    pulsePatternButton->setBounds (143, 8, 59, 20);
    pulsePatternButton->addListener (this);
    pulsePatternButton->setToggleState (true, false);
    addAndMakeVisible (pulsePatternButton.get());

    sinePatternButton = std::make_unique<UtilityButton> ("Sine");
    sinePatternButton->setCorners (false, false, false, false);
    sinePatternButton->setBounds (202, 8, 59, 20);
    sinePatternButton->addListener (this);
    addAndMakeVisible (sinePatternButton.get());

    customPatternButton = std::make_unique<UtilityButton> ("Custom");
    customPatternButton->setCorners (false, true, false, true);
    customPatternButton->setBounds (261, 8, 59, 20);
    customPatternButton->addListener (this);
    addAndMakeVisible (customPatternButton.get());

    currentPattern = new Pattern();
    currentPattern->id = nextPatternId++;

    availablePatterns.add (currentPattern);

    updatePatternSelector();

    PulsePatternGenerator* ppg = new PulsePatternGenerator (this, currentPattern);
    ppg->buildWaveform();

    updateWaveform();

    delete ppg;
}

HarpQuacPlayer::~HarpQuacPlayer()
{
}

void HarpQuacPlayer::resized()
{
    background->setBounds (0, 0, getWidth(), getHeight());
}

void HarpQuacPlayer::setTriggerChannel (int triggerChannel)
{
    currentPattern->triggerChannel = triggerChannel;

    triggerSelector->setSelectedId (triggerChannel + 2);
}

void HarpQuacPlayer::setMode (WaveOutputMode mode)
{
    const int id = mode == WaveOutputMode::LISTEN
                       ? (int) WaveOutputMode::LISTEN
                       : (int) WaveOutputMode::GENERATE;

    if (modeSelector->getSelectedId() == id)
        return;

    modeSelector->setSelectedId (id, dontSendNotification);

    if (onModeChanged)
        onModeChanged (getMode());
}

void HarpQuacPlayer::setAnalogOutputChannel (int channel)
{
    if (currentPattern == nullptr)
        return;

    channel = jlimit (0, kNumAoChannels - 1, channel);
    currentPattern->analogOutputChannel = channel;
    outputChannelSelector->setSelectedId (channel + 1, dontSendNotification);

    if (onWaveformChanged)
        onWaveformChanged();
}

void HarpQuacPlayer::applyPatternType (PatternType type)
{
    if (currentPattern == nullptr)
        return;

    currentPattern->patternType = type;
    selectPatternType (type);

    AnalogPatternGenerator* generator = nullptr;

    if (type == PatternType::pulse)
        generator = new PulsePatternGenerator (this, currentPattern);
    else if (type == PatternType::sine)
        generator = new SinePatternGenerator (this, currentPattern);
    else
        generator = new CustomPatternGenerator (this, currentPattern);

    generator->buildWaveform();
    delete generator;

    updateWaveform();
}

float HarpQuacPlayer::getSampleRate()
{
    // sample rate fixed at 30 kHz
    return 30000.0f;
}

WaveOutputMode HarpQuacPlayer::getMode() const
{
    const int id = modeSelector->getSelectedId();

    if (id == (int) WaveOutputMode::LISTEN)
        return WaveOutputMode::LISTEN;

    return WaveOutputMode::GENERATE;
}

void HarpQuacPlayer::updateWaveform()
{
    LOGD ("Updating waveform for ", currentPattern->name);

    if (getMode() == WaveOutputMode::GENERATE)
        background->updateCurrentWaveform (currentPattern);

    if (onPreviewUpdated)
        onPreviewUpdated();

    if (onWaveformChanged)
        onWaveformChanged();
}

void HarpQuacPlayer::previewListenWaveform (const Array<float>& samples, float maxVoltage, PatternType type)
{
    selectPatternType (type);
    background->updateCurrentWaveform (samples, maxVoltage);

    if (onPreviewUpdated)
        onPreviewUpdated();
}

void HarpQuacPlayer::updatePatternSelector()
{
    patternSelector->clear();

    LOGD ("Updating pattern selector.")

    for (auto pattern : availablePatterns)
    {
        LOGD("  Adding pattern: ", pattern->name);
        patternSelector->addItem (pattern->name, pattern->id);
    }

    patternSelector->addItem ("Add new pattern...", 9999);
    patternSelector->setSelectedId (currentPattern->id, dontSendNotification);
}

void HarpQuacPlayer::comboBoxChanged (ComboBox* comboBox)
{
    if (comboBox == modeSelector.get())
    {
        if (onModeChanged)
            onModeChanged (getMode());

        return;
    }

    if (comboBox == patternSelector.get())
    {
        if (comboBox->getSelectedId() == 0)
        {
            currentPattern->name = patternSelector->getText();

            updatePatternSelector();
        }

        else if (comboBox->getSelectedId() == 9999) // add new pattern
        {
            currentPattern = new Pattern();
            currentPattern->id = nextPatternId++;
            currentPattern->name = "PATTERN " + String (currentPattern->id);
            availablePatterns.add (currentPattern);
            PulsePatternGenerator* ppg = new PulsePatternGenerator (this, currentPattern);
            ppg->buildWaveform();
            delete ppg;

            updatePatternSelector();
            selectPatternType (currentPattern->patternType);
            updateWaveform();

            for (int i = 0; i < 16; i++)
            {
                bool triggerTaken = false;

                for (auto pattern : availablePatterns)
                {
                    if (pattern->triggerChannel == i)
                    {
                        triggerTaken = true;
                    }
                }

                if (! triggerTaken)
                {
                    currentPattern->triggerChannel = i;
                    break;
                }
            }

            for (int i = 0; i < kNumAoChannels; i++)
            {
                bool channelTaken = false;

                for (auto pattern : availablePatterns)
                {
                    if (pattern != currentPattern && pattern->analogOutputChannel == i)
                        channelTaken = true;
                }

                if (! channelTaken)
                {
                    currentPattern->analogOutputChannel = i;
                    break;
                }
            }

            outputChannelSelector->setSelectedId (currentPattern->analogOutputChannel + 1, dontSendNotification);
        }
        else
        {
            for (auto pattern : availablePatterns)
            {
                if (pattern->id == comboBox->getSelectedId())
                    currentPattern = pattern;
            }

            initializePattern (currentPattern);
        }
    }
    else if (comboBox == triggerSelector.get())
    {
        currentPattern->triggerChannel = triggerSelector->getSelectedId() - 2;
    }
    else if (comboBox == outputChannelSelector.get())
    {
        currentPattern->analogOutputChannel = outputChannelSelector->getSelectedId() - 1;

        if (onWaveformChanged)
            onWaveformChanged();
    }
}

void HarpQuacPlayer::selectPatternType (PatternType t)
{
    LOGD ("Selecting pattern type: ", (int) t);

    selectedPatternType = t;

    if (t == PatternType::pulse)
    {
        pulsePatternButton->setToggleState (true, false);
        sinePatternButton->setToggleState (false, false);
        customPatternButton->setToggleState (false, false);
    }
    else if (t == PatternType::sine)
    {
        pulsePatternButton->setToggleState (false, false);
        sinePatternButton->setToggleState (true, false);
        customPatternButton->setToggleState (false, false);
    }
    else if (t == PatternType::custom)
    {
        pulsePatternButton->setToggleState (false, false);
        sinePatternButton->setToggleState (false, false);
        customPatternButton->setToggleState (true, false);
    }
}

void HarpQuacPlayer::initializePattern (Pattern* pattern)
{
    currentPattern = pattern;

    LOGD ("Initializing pattern.");

    if (pattern->patternType == PatternType::pulse)
    {
        auto* patternGenerator = new PulsePatternGenerator (this, pattern);

        patternGenerator->buildWaveform();

        LOGD ("Creating pulse wave.");

        delete patternGenerator;
    }
    else if (pattern->patternType == PatternType::sine)
    {
        auto* patternGenerator = new SinePatternGenerator (this, pattern);

        patternGenerator->buildWaveform();

        LOGD ("Creating sine wave.");

        delete patternGenerator;
    }
    else if (pattern->patternType == PatternType::custom)
    {
        auto* patternGenerator = new CustomPatternGenerator (this, pattern);

        patternGenerator->buildWaveform();

        LOGD ("Creating custom wave.");

        delete patternGenerator;
    }

    updatePatternSelector();
    selectPatternType (pattern->patternType);
    outputChannelSelector->setSelectedId (pattern->analogOutputChannel + 1, dontSendNotification);

    updateWaveform();
}

void HarpQuacPlayer::buttonClicked (Button* button)
{
    if (button == enableButton.get())
    {
        if (enableButton->getToggleState())
        {
            enableButton->setToggleState (false, false);
            enableButton->setLabel ("DISABLED");
            startStopButton->setEnabled (false);
            triggerSelector->setEnabled (false);
        }
        else
        {
            enableButton->setToggleState (true, false);
            enableButton->setLabel ("ENABLED");
            startStopButton->setEnabled (true);
            triggerSelector->setEnabled (true);
        }

        if (onEnabledChanged)
            onEnabledChanged (enableButton->getToggleState());
    }
    else if (button == pulsePatternButton.get()
             || button == sinePatternButton.get()
             || button == customPatternButton.get())
    {
        PatternType clickedType = PatternType::pulse;

        if (button == sinePatternButton.get())
            clickedType = PatternType::sine;
        else if (button == customPatternButton.get())
            clickedType = PatternType::custom;

        const bool alreadySelected = (selectedPatternType == clickedType);

        if (! alreadySelected)
        {
            selectPatternType (clickedType);

            if (getMode() == WaveOutputMode::GENERATE)
            {
                currentPattern->patternType = clickedType;

                if (clickedType == PatternType::pulse)
                {
                    auto* ppg = new PulsePatternGenerator (this, currentPattern);
                    ppg->buildWaveform();
                    delete ppg;
                }
                else if (clickedType == PatternType::sine)
                {
                    auto* spg = new SinePatternGenerator (this, currentPattern);
                    spg->buildWaveform();
                    delete spg;
                }
                else
                {
                    auto* cpg = new CustomPatternGenerator (this, currentPattern);
                    cpg->buildWaveform();
                    delete cpg;
                }

                updateWaveform();
            }

            return;
        }

        // Already selected: open settings popup (read-only in LISTEN mode)
        AnalogPatternGenerator* patternGenerator = nullptr;

        if (clickedType == PatternType::pulse)
            patternGenerator = new PulsePatternGenerator (this, currentPattern);
        else if (clickedType == PatternType::sine)
            patternGenerator = new SinePatternGenerator (this, currentPattern);
        else
            patternGenerator = new CustomPatternGenerator (this, currentPattern);

        patternGenerator->buildWaveform();

        if (getMode() == WaveOutputMode::LISTEN)
            patternGenerator->setReadOnly (true);
        else
            updateWaveform();

        selectPatternType (clickedType);

        CallOutBox::launchAsynchronously (std::unique_ptr<Component> (patternGenerator),
                                          button->getScreenBounds(),
                                          nullptr);
    }
    else if (button == startStopButton.get())
    {
        if (onRunRequested)
            onRunRequested();

        startStopButton->setToggleState (true, false);
        startTimer (currentPattern->samples.size() / getSampleRate() * 1000);
    }
}

void HarpQuacPlayer::timerCallback()
{
    startStopButton->setToggleState (false, false);
}

void HarpQuacPlayer::saveCustomParameters (XmlElement* xml)
{

    XmlElement* xmlNode = xml->createNewChildElement ("WAVEPLAYER");
    xmlNode->setAttribute ("enabled", enableButton->getToggleState());
    xmlNode->setAttribute ("mode", (int) getMode());

    for (auto pattern : availablePatterns)
    {
        XmlElement* xmlNode = xml->createNewChildElement ("PATTERN");

        xmlNode->setAttribute ("id", pattern->id);
        xmlNode->setAttribute ("name", pattern->name);
        xmlNode->setAttribute ("analog_output_channel", pattern->analogOutputChannel);
        xmlNode->setAttribute ("trigger_channel", pattern->triggerChannel);
        xmlNode->setAttribute ("gate_channel", pattern->gateChannel);
        xmlNode->setAttribute ("pulse_on_duration", pattern->pulse.onDuration);
        xmlNode->setAttribute ("pulse_off_duration", pattern->pulse.offDuration);
        xmlNode->setAttribute ("pulse_delay_duration", pattern->pulse.delayDuration);
        xmlNode->setAttribute ("pulse_repeat_number", pattern->pulse.repeatNumber);

        xmlNode->setAttribute ("pulse_ramp_on_duration", pattern->pulse.rampOnDuration);
        xmlNode->setAttribute ("pulse_ramp_off_duration", pattern->pulse.rampOffDuration);
        xmlNode->setAttribute ("pulse_max_voltage", pattern->pulse.maxVoltage);

        xmlNode->setAttribute ("sine_frequency", pattern->sine.frequency);
        xmlNode->setAttribute ("sine_cycles", pattern->sine.cycles);
        xmlNode->setAttribute ("sine_delay_duration", pattern->sine.delayDuration);
        xmlNode->setAttribute ("sine_max_voltage", pattern->sine.maxVoltage);

        xmlNode->setAttribute ("custom_value_string", pattern->custom.string);

        switch (pattern->patternType)
        {
            case PatternType::pulse:
                xmlNode->setAttribute ("pattern_type", 0);
                break;
            case PatternType::sine:
                xmlNode->setAttribute ("pattern_type", 1);
                break;
            case PatternType::custom:
                xmlNode->setAttribute ("pattern_type", 2);
                break;
            default:
                xmlNode->setAttribute ("pattern_type", 0);
        }

        xmlNode->setAttribute ("is_current_pattern", pattern == currentPattern);
    }
}

void HarpQuacPlayer::loadCustomParameters (XmlElement* xml)
{
    availablePatterns.clear();

    bool foundPattern = false;

    forEachXmlChildElement (*xml, xmlNode)
    {
        if (xmlNode->hasTagName("WAVEPLAYER"))
        {
            const int modeId = xmlNode->getIntAttribute ("mode", (int) WaveOutputMode::GENERATE);
            modeSelector->setSelectedId (modeId == (int) WaveOutputMode::LISTEN
                                             ? (int) WaveOutputMode::LISTEN
                                             : (int) WaveOutputMode::GENERATE,
                                         dontSendNotification);

            if (xmlNode->getBoolAttribute ("enabled", false))
                buttonClicked (enableButton.get());
        }
            

        if (xmlNode->hasTagName ("PATTERN"))
        {
            foundPattern = true;
            Pattern* newPattern = new Pattern();

            newPattern->id = xmlNode->getIntAttribute ("id");
            newPattern->name = xmlNode->getStringAttribute ("name");

            LOGD ("Loading pattern ", newPattern->name);
            newPattern->analogOutputChannel = xmlNode->getIntAttribute ("analog_output_channel", 0);
            newPattern->triggerChannel = xmlNode->getIntAttribute ("trigger_channel", 0);
            newPattern->gateChannel = xmlNode->getIntAttribute ("gate_channel", -1);

            newPattern->pulse.onDuration = xmlNode->getIntAttribute ("pulse_on_duration", 100);
            newPattern->pulse.offDuration = xmlNode->getIntAttribute ("pulse_off_duration", 100);
            newPattern->pulse.delayDuration = xmlNode->getIntAttribute ("pulse_delay_duration", 0);
            newPattern->pulse.repeatNumber = xmlNode->getIntAttribute ("pulse_repeat_number", 1);
            newPattern->pulse.rampOnDuration = xmlNode->getIntAttribute ("pulse_ramp_on_duration", 0);
            newPattern->pulse.rampOffDuration = xmlNode->getIntAttribute ("pulse_ramp_off_duration", 0);
            newPattern->pulse.maxVoltage = xmlNode->getDoubleAttribute ("pulse_max_voltage", 5.0);

            newPattern->sine.frequency = xmlNode->getIntAttribute ("sine_frequency", 5);
            newPattern->sine.cycles = xmlNode->getIntAttribute ("sine_cycles", 1);
            newPattern->sine.delayDuration = xmlNode->getIntAttribute ("sine_delay_duration", 0);
            newPattern->sine.maxVoltage = xmlNode->getDoubleAttribute ("sine_max_voltage", 5.0);

            newPattern->custom.string = xmlNode->getStringAttribute ("custom_value_string", "");

            int patternTypeId = xmlNode->getIntAttribute ("pattern_type", 0);

            LOGD ("Pattern type: ", patternTypeId);

            switch (patternTypeId)
            {
                case 0:
                    newPattern->patternType = PatternType::pulse;
                    break;
                case 1:
                    newPattern->patternType = PatternType::sine;
                    break;
                case 2:
                    newPattern->patternType = PatternType::custom;
                    break;
                default:
                    newPattern->patternType = PatternType::pulse;
            }

            if (xmlNode->getBoolAttribute ("is_current_pattern"))
            {
                currentPattern = newPattern;
                LOGD ("Setting as current pattern");
            }

            availablePatterns.add (newPattern);

            if (newPattern->id > nextPatternId)
            {
                nextPatternId = newPattern->id + 1;
            }
        }
    }

    if (foundPattern)
        initializePattern (currentPattern);
}
