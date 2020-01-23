// AMESimParser.cpp : main project file.

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <Windows.h>

using namespace System;
using namespace System::IO::Ports;

// Vehicle parameters, Sky-Hook for adaptive suspension
class pojazd 
{
private:
	std::vector <std::vector <double>> tablica_v1, tablica_v2;
	std::vector <double> tablica_v12;
	std::vector <int> tablica_damp;

public:
	void set_tab(std::vector <std::vector <double>> vec, int nr);
	void disp_tab_elem(int x, int y);
	void auto_SH(void);
	int tab_damp_size(void);
	int tab_damp_ret(int nr);

};

void pojazd::set_tab(std::vector <std::vector <double>> vec, int nr)
{
	if (nr == 1)
	{
		tablica_v1 = vec;
	}
	else if (nr == 2)
	{
		tablica_v2 = vec;
	}
}

void pojazd::disp_tab_elem(int x, int y)
{
	std::cout << tablica_v1[x][y] << " " << tablica_v1[100][1] << std::endl << std::endl;
	std::cout << tablica_v1.size();
}

void pojazd::auto_SH(void)
{
	for (int i = 0; i < tablica_v1.size(); i++)
	{

		tablica_v12.push_back(tablica_v1[i][1] - tablica_v2[i][1]);

		if (tablica_v1[i][1] * tablica_v12[i] > 0 && abs(tablica_v1[i][1]) > 0.8)
		{
			int x = 2;
			tablica_damp.push_back(x);
		}
		else if (tablica_v1[i][1] * tablica_v12[i] > 0)
		{
			int x = 1;
			tablica_damp.push_back(x);
		}
		else
		{
			int x = 0;
			tablica_damp.push_back(x);
		}

	}
}

int pojazd::tab_damp_size(void)
{
	return tablica_damp.size();
}

int pojazd::tab_damp_ret(int nr)
{
	return tablica_damp[nr];
}


// AMESim file read-in, processing
class AMESignal 
{
private:
	int v1_mz; // Velocity of suspended mass
	int v2_mnz; // Velocity of unsuspended mass
	std::vector < std::vector <double> > tablica_dyn;
	std::vector < std::vector <double> > tablica_dyn2;

public:
	void read_file(std::string nazwa, int nr);

	std::vector<std::vector<double>> send_tab();
	void tab_clear(void);

	AMESignal();
	~AMESignal();
};

void AMESignal::read_file(std::string nazwa, int nr)
{
	std::string linijka;

	std::ifstream file(nazwa);
	if (file.is_open())
	{
		while (std::getline(file, linijka))
		{

			std::string::size_type size;
			std::string::size_type memo;
			std::vector <double> wiersz;
			size = linijka.length();
			double licz;

			for (int i = 0; i < size; i = i + memo)
			{
				if (i == 0)
				{
					licz = std::stod(linijka, &memo);
				}
				else
				{
					licz = std::stod(linijka.substr(i), &memo);
				}
				wiersz.push_back(licz);
			}
			if (nr == 1)
			{
				tablica_dyn.push_back(wiersz);
			}
			else if (nr == 2)
			{
				tablica_dyn2.push_back(wiersz);
			}
		}
		file.close();
	}
}

std::vector<std::vector<double>> AMESignal::send_tab()
{
	return tablica_dyn;
}

void AMESignal::tab_clear(void)
{
	tablica_dyn.clear();
}

AMESignal::AMESignal(void)
{
	std::cout << "Created" << std::endl;
}
AMESignal::~AMESignal()
{
	std::cout << "Deleted" << std::endl;
}


int main(array<System::String ^> ^args)
{
	pojazd Golf;
	AMESignal signal;

	signal.read_file("AMEsin1.txt", 1);
	Golf.set_tab(signal.send_tab(), 1);
	signal.tab_clear();
	signal.read_file("AMEsin2.txt", 1);
	Golf.set_tab(signal.send_tab(), 2);
	signal.tab_clear();

	Golf.auto_SH();

	String^ answer;
	String^ portName;
	int baudRate = 57600;

	portName = "COM4";
	SerialPort^ arduino;
	arduino = gcnew SerialPort(portName, baudRate);

	try
	{
		arduino->Open();

		do
		{
			String^ nastawa3 = "";

			for (int i = 0; i < Golf.tab_damp_size(); i++)
			{
				int nastawa = Golf.tab_damp_ret(i);
				String^ nastawa2 = System::Convert::ToString(nastawa);
				
					arduino->Write(nastawa2);
					Console::Write(nastawa2);

				Sleep(10);
			}
			
			Console::WriteLine("Jeszcze raz? y/n");

			answer = Console::ReadLine();

			Console::Clear();
		} while (String::Compare(answer, "y") == 0);

		arduino->Close();
	}
	catch (IO::IOException^ e)
	{
		Console::WriteLine(e->GetType()->Name + ": Port not ready!");
	}
	catch (ArgumentException^ e)
	{
		Console::WriteLine(e->GetType()->Name + ": Wrong COM name!");
	}

	Console::Write("Press ENTER to Exit");
	Console::Read();
	return 0;
}