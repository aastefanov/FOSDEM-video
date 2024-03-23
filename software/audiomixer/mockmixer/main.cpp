#include <string>
#include <iostream>
#include <cstdlib>

class MockSerial{
	public:
		constexpr static auto printf = std::printf;
};


class AudioMixer4 {
	private:
		float _gains[6] = {0,};
	public:
		void gain(int channel, float gain) {
			_gains[channel] = gain;
		}

		float getGain(int channel) {
			return _gains[channel];
		}
		
};

bool echo = false;


std::string cmdbuffer;

AudioMixer4 mixer1;         //xy=787,364
AudioMixer4 mixer4; //xy=788,502
AudioMixer4 mixer2;         //xy=789,428
AudioMixer4 mixer5; //xy=790,566
AudioMixer4 mixer7; //xy=791,640
AudioMixer4 mixer8; //xy=793,704
AudioMixer4 mixer10; //xy=795,773
AudioMixer4 mixer11; //xy=797,837
AudioMixer4 mixer13; //xy=799,913
AudioMixer4 mixer14; //xy=801,977
AudioMixer4 mixer16; //xy=803,1046
AudioMixer4 mixer17; //xy=805,1110
AudioMixer4 mixer3;         //xy=949,395
AudioMixer4 mixer6; //xy=951,536
AudioMixer4 mixer9; //xy=951,673
AudioMixer4 mixer12; //xy=955,803
AudioMixer4 mixer15; //xy=956,948
AudioMixer4 mixer18; //xy=957,1075

AudioMixer4 *matrix[6][2] = {
	{&mixer1,  &mixer2},
	{&mixer4,  &mixer5},
	{&mixer7,  &mixer8},
	{&mixer10, &mixer11},
	{&mixer13, &mixer14},
	{&mixer16, &mixer17},
};

const MockSerial Serial = MockSerial();

void
set_crosspoint(int channel, int bus, float gain)
{
	matrix[bus][channel / 4]->gain(channel % 4, gain);
}

void
set_mix(int bus, float in1, float in2, float in3, float in4, float in5, float in6)
{
	bus--;
	matrix[bus][0]->gain(0, in1);
	matrix[bus][0]->gain(1, in2);
	matrix[bus][0]->gain(2, in3);
	matrix[bus][0]->gain(3, in4);
	matrix[bus][1]->gain(0, in5);
	matrix[bus][1]->gain(1, in6);
	matrix[bus][1]->gain(2, 0.0f);
	matrix[bus][1]->gain(3, 0.0f);
}


void
print_channel_gain(const char *name, int channel)
{
	float gains[6] = {};
	gains[0] = matrix[channel][0]->getGain(0);
	gains[1] = matrix[channel][0]->getGain(1);
	gains[2] = matrix[channel][0]->getGain(2);
	gains[3] = matrix[channel][0]->getGain(3);
	gains[4] = matrix[channel][1]->getGain(0);
	gains[5] = matrix[channel][1]->getGain(1);
	Serial.printf("%-6s", name);
	for (float gain: gains) {
		Serial.printf("%-3d  ", (int) (gain * 100));
	}
	Serial.printf("\r\n");
}

void
do_cmd()
{
	char cmd = cmdbuffer[0];
	switch (cmd) {
		case 'l': {
			int channel = cmdbuffer[1] - '1';
			if (channel > 5 || channel < 0) {
				Serial.printf("usage: l<channel> in range 1-6\r\n");
				return;
			}
			float gains[] = {
				matrix[channel][0]->getGain(0),
				matrix[channel][0]->getGain(1),
				matrix[channel][0]->getGain(2),
				matrix[channel][0]->getGain(3),
				matrix[channel][1]->getGain(0),
				matrix[channel][1]->getGain(1),
			};
			for (float gain: gains) {
				Serial.printf("%d ", (int) (gain * 100));
			}
			Serial.printf("\r\n");
			break;
		}
		case 's': {
			int input = cmdbuffer[1] - '1';
			int output = cmdbuffer[2] - '1';
			int gain = std::stoi(cmdbuffer.substr(3, 3));
			set_crosspoint(input, output, (float) gain / 100.0f);
			Serial.printf("%d -> %d = %d\r\n", input, output, gain);
			break;
		}
		case 'e': {
			// Use e0 and e1 to disable or enable uart echo
			echo = cmdbuffer[1] == '1';
			break;
		}
		case 'h': {
			Serial.printf("Audio mixer commands:\r\n");
			Serial.printf("e<0|1>            set the UART echo state\r\n");
			Serial.printf("e<0|1>            set the UART echo state\r\n");
			Serial.printf("l<out>            print the levels sent to a specific output\r\n");
			Serial.printf("[lf]              print the whole human-readable mixing matrix\r\n");
		}
		case 10:
		case 13:
			// Special case, print human readable status
			Serial.printf("      IN1  IN2  IN3  PC   USB1 USB2\r\n");
			print_channel_gain("OUT1", 0);
			print_channel_gain("OUT2", 1);
			print_channel_gain("HP L", 2);
			print_channel_gain("HP R", 3);
			print_channel_gain("USB1", 4);
			print_channel_gain("USB2", 5);
			break;
		case 27:
		case 91:
			// Catch escape sequences
			break;
		default:
			Serial.printf("cmd: %d\r\n", cmd);
	}
}

void
do_uart()
{
	std::getline(std::cin, cmdbuffer);
	if(cmdbuffer.empty()) cmdbuffer = "\r";

	// note: input is buffered, so echo WILL wait for the line to be completed
	if(echo) std::cout << cmdbuffer << std::endl; // flush!!
	
	do_cmd();
}


void setup() {}

void loop() { do_uart(); }

int
main() {
	setup();

	while(1) {
		loop();
	}
}
