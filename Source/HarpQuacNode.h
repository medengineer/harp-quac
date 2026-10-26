#ifndef HARPQUACNODE_H_DEFINED
#define HARPQUACNODE_H_DEFINED

#include <ProcessorHeaders.h>

#include <atomic>
#include <memory>

class HarpQuacPlayer;

class HarpQuacNode : public GenericProcessor
{
public:
    HarpQuacNode();
    ~HarpQuacNode() override;

    AudioProcessorEditor* createEditor() override;

    void registerParameters() override;
    void updateSettings() override;
    void process (AudioBuffer<float>& buffer) override;

    void handleTTLEvent (TTLEventPtr event) override;
    void handleBroadcastMessage (const String& message, const int64 systemTimeMillis) override;
    String handleConfigMessage (const String& message) override;

    bool startAcquisition() override;
    bool stopAcquisition() override;

    HarpQuacPlayer* getPlayer();

    static StringArray listSerialPorts();
    void setSelectedPort (const String& port);
    String getSelectedPort() const { return selectedPort; }

    void updateArmedState (HarpQuacPlayer* player);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

    String selectedPort;
    std::atomic<int> triggerLine { -1 };
    std::atomic<int> outputMode { 1 };

    void playCurrent (HarpQuacPlayer* player, bool loop);
    void playAll (HarpQuacPlayer* player, bool loop);
    void stopPlayback();
    void replayArmed();
    void wire (HarpQuacPlayer* player);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarpQuacNode);
};

#endif
