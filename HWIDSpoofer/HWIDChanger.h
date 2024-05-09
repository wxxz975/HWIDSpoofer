#pragma once

class HWIDChanger
{
public:
	HWIDChanger() = default;
	~HWIDChanger() = default;

	/// <summary>
	/// Initialize the changer class and search for the required pattern
	/// </summary>
	/// <returns></returns>
	virtual bool Initialize() = 0;

	/// <summary>
	/// execute
	/// </summary>
	/// <returns></returns>
	virtual bool Execute() = 0;


	virtual void ShowAllAddress() = 0;
private:

};

