/*
  ==============================================================================
  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Voice.h"
#include "Note.h"
#include <math.h> 
#include <iostream>

//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/
class MainContentComponent : public AudioAppComponent, Slider::Listener, private MidiInputCallback, private MidiKeyboardStateListener

{
public:
	//==============================================================================
	MainContentComponent()
		: currentSampleRate(0.0), keyboardComponent(keyboardState, MidiKeyboardComponent::horizontalKeyboard),
		startTime(Time::getMillisecondCounterHiRes() * 0.001)
	{
		setSize(800, 400);

		
		addAndMakeVisible(levelSlider);
		levelSlider.setRange(0.0f, 1, 0.001f);
		levelSlider.setValue(0.5f);
		levelSlider.addListener(this);

		addAndMakeVisible(gainSlider);
		gainSlider.setRange(0,5);
		gainSlider.setValue(0);
		gainSlider.addListener(this);

		addAndMakeVisible(fre);
		fre.setRange(0, 1);
		fre.setValue(0);
		fre.addListener(this);

		addAndMakeVisible(levelB);
		levelB.setRange(0, 1);
		levelB.setValue(0.5);
		levelB.addListener(this);

		addAndMakeVisible(levelA);
		levelA.setRange(0, 1);
		levelA.setValue(0.5);
		levelA.addListener(this);

		addAndMakeVisible(del);
		del.setRange(1, 100);
		del.setValue(1);
		del.addListener(this);

		addAndMakeVisible(relSlider);
		relSlider.setRange(0.0001, 2);
		relSlider.setValue(0.1);
		relSlider.addListener(this);

		addAndMakeVisible(atcSlider);
		atcSlider.setRange(0.0001, 2);
		atcSlider.setValue(0.1);
		atcSlider.addListener(this);


		addAndMakeVisible(midiInputListLabel);
		midiInputListLabel.setText("MIDI Input:", dontSendNotification);
		midiInputListLabel.attachToComponent(&midiInputList, true);

		addAndMakeVisible(midiInputList);
		midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
		auto midiInputs = MidiInput::getDevices();
		midiInputList.addItemList(midiInputs, 1);
		midiInputList.onChange = [this] { setMidiInput(midiInputList.getSelectedItemIndex()); };

		// find the first enabled device and use that by default
		for (auto midiInput : midiInputs)
		{
			if (deviceManager.isMidiInputEnabled(midiInput))
			{
				setMidiInput(midiInputs.indexOf(midiInput));
				break;
			}
		}

		// if no enabled devices were found just use the first one in the list
		if (midiInputList.getSelectedId() == 0)
			setMidiInput(0);

		addAndMakeVisible(keyboardComponent);
		keyboardState.addListener(this);

		addAndMakeVisible(midiMessagesBox);
		midiMessagesBox.setMultiLine(true);
		midiMessagesBox.setReturnKeyStartsNewLine(true);
		midiMessagesBox.setReadOnly(true);
		midiMessagesBox.setScrollbarsShown(true);
		midiMessagesBox.setCaretVisible(false);
		midiMessagesBox.setPopupMenuEnabled(true);
		midiMessagesBox.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
		midiMessagesBox.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
		midiMessagesBox.setColour(TextEditor::shadowColourId, Colour(0x16000000));


		// specify the number of input and output channels that we want to open
		setAudioChannels(0, 2);
	}

	~MainContentComponent()
	{
		shutdownAudio();
	}

	//==============================================================================
	void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override
	{
		currentSampleRate = sampleRate; 
		v[0] = Voice(currentSampleRate);
		v[1] = Voice(currentSampleRate);
		v[2] = Voice(currentSampleRate);
		v[3] = Voice(currentSampleRate);
		v[4] = Voice(currentSampleRate);
		v2[0] = Voice(currentSampleRate);
		v2[1] = Voice(currentSampleRate);
		v2[2] = Voice(currentSampleRate);
		v2[3] = Voice(currentSampleRate);
		v2[4] = Voice(currentSampleRate);
		v3[0] = Voice(currentSampleRate);
		v3[1] = Voice(currentSampleRate);
		v3[2] = Voice(currentSampleRate);
		v3[3] = Voice(currentSampleRate);
		v3[4] = Voice(currentSampleRate);
	}


	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		float level = levelSlider.getValue();
		float gain = gainSlider.getValue();
		//DBG(String(gain));
		float la = levelA.getValue();
		float lb = levelB.getValue();
		float f = fre.getValue();
		float d = del.getValue();

		d = currentSampleRate * d / 1000;
		f = f / currentSampleRate;


		float* const buffer0 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		float* const buffer1 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

		for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
		{

			double currentSample = 0;

			for (int i = 0; i < 5; i++) {
				if (note[i].getIfPlaying()) {
					double theNote = key * pow(2, (note[i].getNote() - 1) / 12);
					double noteSample = v[i].getNextSample(theNote);
					noteSample += v2[i].getNextSample(theNote * (100 + gain) / 100);
					noteSample += v3[i].getNextSample(theNote * (100 - gain) / 100);
					noteSample = noteSample * note[i].getNextVol();
					currentSample += noteSample * level;
				}
				
			}
			
			currentSample = filter(currentSample, d,f,la,lb);
			buffer0[sample] = currentSample;
			buffer1[sample] = currentSample;
		}

	}

	int ind = 0;
	double arr[8820] = {0};
	double filter(double sample, float d, float f, float md, float lb) {
		arr[ind] = sample;
		int t = d * (1 + md * std::sin(2 * MathConstants<double>::pi * f * ind));
		int ni = ind - t;
		if (ni < 0) {
			ni += 8820;
		}
		sample += lb * arr[ni];
		
		ind = (ind + 1) % 8820;
		return sample;
	}

	void releaseResources() override
	{
		// This will be called when the audio device stops, or when it is being
		// restarted due to a setting change.

		// For more details, see the help for AudioProcessor::releaseResources()
	}

	//==============================================================================
	void paint(Graphics& g) override
	{
		// (Our component is opaque, so we must completely fill the background with a solid colour)
		g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

		// You can add your drawing code here!
	}

	void resized() override
	{
		levelSlider.setBounds(0, 50, 100, 100);
		gainSlider.setBounds(0, 150, 100, 100);
		midiInputList.setBounds(0, 0, 800, 50);
		keyboardComponent.setBounds(0, 250, 800, 100);
		midiMessagesBox.setBounds(0, 350, 800, 50);
		del.setBounds(400, 50, 100, 100);
		fre.setBounds(500, 50, 100, 100);
		levelA.setBounds(600, 50, 100, 100);
		levelB.setBounds(700, 50, 100, 100);
		atcSlider.setBounds(500, 150, 100, 100);
		relSlider.setBounds(600, 150, 100, 100);
	}

	void sliderValueChanged(Slider * slider) override
	{
		int r = relSlider.getValue();
		int a = atcSlider.getValue();
		for (int i = 0; i < 5; i++) {
			note[i].setASR(a, r);
		}
	}


private:
	
	static String getMidiMessageDescription(const MidiMessage& m)
	{
		if (m.isNoteOn())           return "Note on " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
		if (m.isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
		if (m.isProgramChange())    return "Program change " + String(m.getProgramChangeNumber());
		if (m.isPitchWheel())       return "Pitch wheel " + String(m.getPitchWheelValue());
		if (m.isAftertouch())       return "After touch " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + String(m.getAfterTouchValue());
		if (m.isChannelPressure())  return "Channel pressure " + String(m.getChannelPressureValue());
		if (m.isAllNotesOff())      return "All notes off";
		if (m.isAllSoundOff())      return "All sound off";
		if (m.isMetaEvent())        return "Meta event";

		if (m.isController())
		{
			String name(MidiMessage::getControllerName(m.getControllerNumber()));

			if (name.isEmpty())
				name = "[" + String(m.getControllerNumber()) + "]";

			return "Controller " + name + ": " + String(m.getControllerValue());
		}

		return String::toHexString(m.getRawData(), m.getRawDataSize());
	}

	void logMessage(const String& m)
	{
		midiMessagesBox.moveCaretToEnd();
		midiMessagesBox.insertTextAtCaret(m + newLine);
	}

	/** Starts listening to a MIDI input device, enabling it if necessary. */
	void setMidiInput(int index)
	{
		auto list = MidiInput::getDevices();

		deviceManager.removeMidiInputCallback(list[lastInputIndex], this);

		auto newInput = list[index];

		if (!deviceManager.isMidiInputEnabled(newInput))
			deviceManager.setMidiInputEnabled(newInput, true);

		deviceManager.addMidiInputCallback(newInput, this);
		midiInputList.setSelectedId(index + 1, dontSendNotification);

		lastInputIndex = index;
	}

	// These methods handle callbacks from the midi device + on-screen keyboard..
	void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message) override
	{
		const ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
		keyboardState.processNextMidiEvent(message);
		postMessageToList(message, source->getName());
	}

	void handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
	{
		if (!isAddingFromMidiInput)
		{
			for (int i = 0; i < 5; i++) {
				if (!note[i].getIfPlaying()) {
					note[i].playNote(midiNoteNumber, Time::getMillisecondCounterHiRes() * 0.001);
					break;
				}
			}
		}
	}

	void handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override
	{
		if (!isAddingFromMidiInput)
		{
			for (int i = 0; i < 5; i++) {
				if (note[i].getNote() == midiNoteNumber && note[i].isKeyDown()) {
					note[i].upNote(Time::getMillisecondCounterHiRes() * 0.001);
					break;
				}
			}
		}
	}

	// This is used to dispach an incoming message to the message thread
	class IncomingMessageCallback : public CallbackMessage
	{
	public:
		IncomingMessageCallback(MainContentComponent* o, const MidiMessage& m, const String& s)
			: owner(o), message(m), source(s)
		{}

		void messageCallback() override
		{
			if (owner != nullptr)
				owner->addMessageToList(message, source);
		}

		Component::SafePointer<MainContentComponent> owner;
		MidiMessage message;
		String source;
	};

	void postMessageToList(const MidiMessage& message, const String& source)
	{
		(new IncomingMessageCallback(this, message, source))->post();
	}

	void addMessageToList(const MidiMessage& message, const String& source)
	{
		auto time = message.getTimeStamp() - startTime;

		auto hours = ((int)(time / 3600.0)) % 24;
		auto minutes = ((int)(time / 60.0)) % 60;
		auto seconds = ((int)time) % 60;
		auto millis = ((int)(time * 1000.0)) % 1000;

		auto timecode = String::formatted("%02d:%02d:%02d.%03d",
			hours,
			minutes,
			seconds,
			millis);

		auto description = getMidiMessageDescription(message);

		String midiMessageString(timecode + "  -  " + description + " (" + source + ")"); // [7]
		logMessage(midiMessageString);
	}
	
	AudioDeviceManager deviceManager;           // [1]
	ComboBox midiInputList;                     // [2]
	Label midiInputListLabel;
	int lastInputIndex = 0;                     // [3]
	bool isAddingFromMidiInput = false;         // [4]

	MidiKeyboardState keyboardState;            // [5]
	MidiKeyboardComponent keyboardComponent;    // [6]

	TextEditor midiMessagesBox;
	double startTime;


	//==============================================================================
	float level, gain;
	//Voice v = NULL;
	/*Voice v2 = NULL;
	Voice v3 = NULL;*/
	const double key = 16.35 / 4;
	double currentSampleRate;//, note;
	Voice v[5] = { NULL , NULL ,NULL ,NULL ,NULL };
	Voice v2[5] = { NULL , NULL ,NULL ,NULL ,NULL };
	Voice v3[5] = { NULL , NULL ,NULL ,NULL ,NULL };
	Note note[5] = {};
	//double note[5] = {-1,-1,-1,-1,-1};
	Slider levelSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
	Slider gainSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };

	Slider fre{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
	Slider del{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
	Slider levelA{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
	Slider levelB{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };

	Slider atcSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };
	Slider relSlider{ Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent() { return new MainContentComponent(); }

