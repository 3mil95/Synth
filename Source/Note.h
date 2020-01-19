/*
  ==============================================================================

    Note.h
    Created: 30 Sep 2019 3:58:08pm
    Author:  emilc

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Note
{
public:
	Note() {
		isPlaying = false;
		keyDown = false;
		note = -1;

		atk = 0.1f;
		dec = 0.1f;
		noteDown = 0;
		noteUp = 0;
	}

	bool isKeyDown() {
		return keyDown;
	}

	bool getIfPlaying() {
		return isPlaying;
	}

	double getNote() {
		return note;
	}

	void playNote(double n, double time) {
		noteDown = time;
		isPlaying = true;
		keyDown = true;
		note = n;
	}

	void upNote(double time) {	
		noteUp = time;
		keyDown = false;
	}

	void noteDone() {
		isPlaying = false;
		noteUp = 0;
		noteDown = 0;
		note = -1;
	}

	void setASR(double A, double D) {
		atk = A;
		dec = D;
	}

	double getNextVol() {
		double timeNow = Time::getMillisecondCounterHiRes() * 0.001;
		double DTime;
		double vol;
		noteDown++;
		if (keyDown == true) {
			DTime = noteDown - timeNow;
			// A
			if (DTime < atk) {
				return (DTime / atk);
			}
			else
			{
				vol = 1;
			}
		}
		else {
			// R
			DTime = timeNow - noteUp;
			vol = ((dec - DTime) / dec);
			if (vol <= 0) {
				vol = 0;
				noteDone();
			}
		}
		
		return vol;
		
	}

private:
	bool isPlaying;
	bool keyDown;
	double note;
	double noteDown;
	double noteUp;
	double index;

	double atk;
	double dec;
};
