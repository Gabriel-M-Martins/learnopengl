#pragma once

#include <functional>

struct KeySettings {
	int key;
	std::function<void()> func;
	bool shouldRepeat = false;
	float cooldown = 0.0f;
	float lastTriggerTime = 0.0f;
};
