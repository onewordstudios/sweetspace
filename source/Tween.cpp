#include "Tween.h"

/** Max value of a byte */
constexpr uint8_t MAX_BYTE = 255;

float Tween::linInterp(float start, float end, float percentage) {
	return start * (1.0f - percentage) + end * percentage;
}

float Tween::linear(float start, float end, int currFrame, int maxFrame) {
	return linInterp(start, end, (float)currFrame / (float)maxFrame);
}

float Tween::easeIn(float start, float end, int currFrame, int maxFrame) {
	float t = (float)currFrame / (float)maxFrame;
	return linInterp(start, end, t * t * t * t);
}

float Tween::easeOut(float start, float end, int currFrame, int maxFrame) {
	float t = (float)currFrame / (float)maxFrame;
	t--;
	return linInterp(start, end, -t * t * t * t + 1);
}

cugl::Color4 Tween::fade(float a) {
	return cugl::Color4(MAX_BYTE, MAX_BYTE, MAX_BYTE, (uint8_t)(MAX_BYTE * a));
}
