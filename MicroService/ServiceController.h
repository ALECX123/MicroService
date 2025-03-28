#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <xstring>
#include "Micro_base.h"
class ServiceController
{
public:
	ServiceController();

	ServiceController(ServiceController&& other) noexcept;
	ServiceController& operator=(ServiceController&& other) noexcept;

	virtual ~ServiceController();

	bool open(std::u16string_view name);
	bool install(std::u16string_view name,
		std::u16string_view display_name,
		const std::filesystem::path& file_path);
	bool remove(std::u16string_view name);
	bool isInstalled(std::u16string_view name);
	bool isRunning(std::u16string_view name);

	void close();

	bool setDescription(std::u16string_view description);
	std::u16string description() const;

	bool setDependencies(const std::vector<std::u16string>& dependencies);
	std::vector<std::u16string> dependencies() const;

	bool setAccount(std::u16string_view username, std::u16string_view password);

	std::filesystem::path filePath() const;

	bool isValid() const;
	bool isRunning() const;

	bool start();
	bool stop();


protected:
	ServiceController(SC_HANDLE sc_manager, SC_HANDLE service);

private:
	ScopedScHandle sc_manager_;
	mutable ScopedScHandle service_;

	DISALLOW_COPY_AND_ASSIGN(ServiceController);
};

