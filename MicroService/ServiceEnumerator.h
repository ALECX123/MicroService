#ifndef SERVICEENUMERATOR_H
#define SERVICEENUMERATOR_H
#include <cstdint>
#include <memory>
#include <string>
class ServiceEnumerator
{
public:
	enum class Type { SERVICES = 0, DRIVERS = 1 };

	explicit ServiceEnumerator(Type type);
	~ServiceEnumerator() = default;

	bool isAtEnd() const;
	void advance();

	std::wstring nameW() const;
	std::string name() const;

	std::wstring displayNameW() const;
	std::string displayName() const;

	std::wstring descriptionW() const;
	std::string description() const;

	enum class Status
	{
		UNKNOWN = 0,
		CONTINUE_PENDING = 1,
		PAUSE_PENDING = 2,
		PAUSED = 3,
		RUNNING = 4,
		START_PENDING = 5,
		STOP_PENDING = 6,
		STOPPED = 7
	};

	Status status() const;

	enum class StartupType
	{
		UNKNOWN = 0,
		AUTO_START = 1,
		DEMAND_START = 2,
		DISABLED = 3,
		BOOT_START = 4,
		SYSTEM_START = 5
	};

	StartupType startupType() const;

	std::wstring binaryPathW() const;
	std::string binaryPath() const;

	std::wstring startNameW() const;
	std::string startName() const;
};


#endif // SERVICEENUMERATOR


