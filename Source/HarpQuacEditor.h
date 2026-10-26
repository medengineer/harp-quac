/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2026 Open Ephys

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

#ifndef HARPQUACEDITOR_H_DEFINED
#define HARPQUACEDITOR_H_DEFINED

#include <EditorHeaders.h>

class HarpQuacPlayer;
class WaveformView;

/**

	Editor for the HarpQuacNode.

	Same layout as NIDAQ Output, without Mirror mode or the device-settings button.
	The HarpQuacPlayer pattern model stays hidden and keeps the existing playback callbacks.

*/
class HarpQuacEditor : public GenericEditor,
                         public Button::Listener,
                         public ComboBox::Listener,
                         private Timer
{
public:
    /** Constructor */
    HarpQuacEditor (GenericProcessor* parentNode);

    /** Destructor */
    ~HarpQuacEditor() override;

    /** Returns the HarpQuacPlayer pattern editor */
    HarpQuacPlayer* getHarpQuacPlayer() const { return wavePlayer.get(); }

    /** Saves HarpQuacPlayer pattern parameters */
    void saveCustomParametersToXml (XmlElement* xml) override;

    /** Loads HarpQuacPlayer pattern parameters */
    void loadCustomParametersFromXml (XmlElement* xml) override;

    void startAcquisition() override;
    void stopAcquisition() override;

private:
    void buttonClicked (Button* button) override;
    void comboBoxChanged (ComboBox* comboBox) override;
    void timerCallback() override;

    void addParameterRow (const String& name,
                          std::unique_ptr<Label>& label,
                          std::unique_ptr<ComboBox>& combo,
                          int y);

    void updateConfigUI();
    void refreshDeviceSelector();
    void updateDeviceSelectorEnabled();
    void refreshWaveformView();
    void openWaveformSettings (Button* button);
    void storeTrigger();
    void syncTriggerSelector();
    void applyTriggerAttribute (const String& trigger);
    void syncConfigFromPlayer();

    std::unique_ptr<HarpQuacPlayer> wavePlayer;

    std::unique_ptr<Label> deviceLabel;
    std::unique_ptr<Label> configLabel;
    std::unique_ptr<Label> triggerLabel;
    std::unique_ptr<Label> outputLabel;
    std::unique_ptr<Label> waveformLabel;

    std::unique_ptr<ComboBox> deviceSelector;
    std::unique_ptr<ComboBox> configSelector;
    std::unique_ptr<ComboBox> triggerSelector;
    std::unique_ptr<ComboBox> outputSelector;
    std::unique_ptr<ComboBox> waveformSelector;

    std::unique_ptr<UtilityButton> settingsButton;
    std::unique_ptr<WaveformView> waveformView;

    String deviceSelectorSignature;
    bool deviceSelectorHasChoice = false;
    bool acquisitionIsActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarpQuacEditor);
};

#endif // HARPQUACEDITOR_H_DEFINED
