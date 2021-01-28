#include "Tween.h"

#include <cmath>

/** Max value of a byte */
constexpr uint8_t MAX_BYTE = 255;

float Tween::linInterp(float start, float end, float percentage) {
	return start * (1.0f - percentage) + end * percentage;
}

float Tween::linear(float start, float end, size_t currFrame, size_t maxFrame) {
	return linInterp(start, end, static_cast<float>(currFrame) / static_cast<float>(maxFrame));
}

float Tween::easeIn(float start, float end, size_t currFrame, size_t maxFrame) {
	float t = static_cast<float>(currFrame) / static_cast<float>(maxFrame);
	return linInterp(start, end, t * t * t * t);
}

float Tween::easeOut(float start, float end, size_t currFrame, size_t maxFrame) {
	float t = static_cast<float>(currFrame) / static_cast<float>(maxFrame);
	t--;
	return linInterp(start, end, -t * t * t * t + 1);
}

float Tween::easeInOut(float start, float end, size_t currFrame, size_t maxFrame) {
	const float t = static_cast<float>(currFrame) / static_cast<float>(maxFrame);
	const float halfPos = ((end - start) / 2) + start;
	const size_t halfFrame = maxFrame / 2;
	if (2 * t < 1) {
		return easeIn(start, halfPos, currFrame, halfFrame);
	}
	return easeOut(halfPos, end, currFrame - halfFrame, halfFrame);
}

float Tween::loop(size_t currFrame, size_t maxFrame) {
	return (1 - cosf(2 * static_cast<float>(M_PI) * currFrame / maxFrame)) / 2;
}

cugl::Color4 Tween::fade(float a) {
	return {MAX_BYTE, MAX_BYTE, MAX_BYTE, static_cast<uint8_t>(MAX_BYTE * a)};
}
