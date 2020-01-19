/*
  ==============================================================================

    Voice.h
    Created: 28 Sep 2019 7:46:34pm
    Author:  emilc

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <math.h> 


class Voice {
public:
	
	Voice(double sampleRate) {
		currentSampleRate = sampleRate;
		createTable();
	}

	double getSampleValue() {
		int i = (int)index;
		i = i % tableSize;
		return waveTable[i];
	}

	void restartIndex() {
		index = 0;
	}

	double getNextSample(double note) {

		double next = getSampleValue();
		index += note * (tableSize / currentSampleRate);
		if (index >= tableSize) {
			index -= tableSize;
		}
		return next;
	}

	void createTable() {
		double angleDelta = MathConstants<double>::twoPi / (double)(tableSize); // [4]
		double currentAngle = 0.0;

		for (auto i = 0; i < tableSize; ++i)
		{
			double sample = std::sin(currentAngle);     // [5]
			waveTable[i] = (double)sample;
			currentAngle += angleDelta;
		}
		/*for (int i = 0; i < tableSize; i++)
		{
			if (i > tableSize / 2) {
				waveTable[i] = -0.125f;
			}
			else
			{
				waveTable[i] = 0.125f;
			}

		}*/
	}

private:
	double currentSampleRate;
	int tableSize = 2048;
	double index = 0;
	double waveTable[2048];
};
