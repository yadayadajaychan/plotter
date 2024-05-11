// Copyright (C) 2024 Ethan Cheng <ethan@nijika.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <DS1803.h>
#include "gcode.h"
#include <math.h>

unsigned char CURR_X;
unsigned char CURR_Y;
float STEP = 2;
unsigned int FEED_RATE;

char line_buffer[256];
GCode gcode(line_buffer, 256);

void setX(unsigned char x);
void setY(unsigned char y);

// Rapid Positioning
void G00(unsigned char x, unsigned char y);
// Linear Interpolation
void G01(unsigned char x, unsigned char y);

// Circular Interpolation Clockwise
void G02(unsigned char x, unsigned char y, short i, short j);
// Circular Interpolation Counterclockwise
void G03(unsigned char x, unsigned char y, short i, short j);

float distance(unsigned char x1, unsigned char y1,
               unsigned char x2, unsigned char y2);

float angle(unsigned char x1, unsigned char y1,
            unsigned char x2, unsigned char y2);

void parse();

void setup()
{
	initDS1803();
	G00(0, 0);
	Serial.begin(9600);
	Serial.setTimeout(1000);
	Serial.println("READY");
}

void loop()
{
	if (Serial.available() > 0) {
		if (gcode.addChar(Serial.read())) {
			parse();
			gcode.reset();
		}
	}
}

void setX(unsigned char x)
{
	setWiper(A_000, WIPER_1, x);
	CURR_X = x;
}

void setY(unsigned char y)
{
	setWiper(A_000, WIPER_0, y);
	CURR_Y = y;
}

void G00(unsigned char x, unsigned char y)
{
	setX(x);
	setY(y);
}

void G01(unsigned char x, unsigned char y)
{
	float dist = distance(CURR_X, CURR_Y, x, y);
	float ang = angle(CURR_X, CURR_Y, x, y);

	int new_x, new_y;
	float dx, dy;
	while (dist > STEP) {
		dx = roundf(STEP * cosf(ang));
		dy = roundf(STEP * sinf(ang));

		new_x = CURR_X + dx;
		new_y = CURR_Y + dy;

		// check for overflow/underflow
		if (new_x > 255)
			new_x = 255;
		if (new_y > 255)
			new_y = 255;
		if (new_x < 0)
			new_x = 0;
		if (new_y < 0)
			new_y = 0;
			
		G00(new_x, new_y);
		dist = distance(CURR_X, CURR_Y, x, y);
		ang = angle(CURR_X, CURR_Y, x, y);
	}

	G00(x, y);
	//TODO calculate time delay
	return;
}
	
float distance(unsigned char x1, unsigned char y1,
               unsigned char x2, unsigned char y2)
{
	float dx = (float) x2 - x1;
	float dy = (float) y2 - y1;
	return sqrtf(dx*dx + dy*dy);
}

float angle(unsigned char x1, unsigned char y1,
            unsigned char x2, unsigned char y2)
{
	float dx = (float) x2 - x1;
	float dy = (float) y2 - y1;

	if (dx == 0) {
		if (dy == 0)
			return 0;
		else if (dy > 0)
			return M_PI_2;
		else
			return M_PI + M_PI_2;
	} else if (dx > 0) {
		if (dy >= 0)
			return atanf(dy/dx);
		else
			return 2*M_PI - atanf(-dy/dx);
	} else {
		if (dy >= 0)
			return M_PI - atanf(-dy/dx);
		else
			return M_PI + atanf(dy/dx);
	}
}

void parse()
{
	if (!gcode.exists('G')) {
		Serial.println("ERR: UNIMPLEMENTED GCODE");
		return;
	}

	if (!gcode.exists('X') || !gcode.exists('Y')) {
		Serial.println("ERR: MISSING X OR Y COORDINATE");
		return;
	}

	int x = round(gcode.get('X'));
	int y = round(gcode.get('Y'));

	if (x < 0 || x > 255 || y < 0 || y > 255) {
		Serial.println("ERR: INVALID COORDINATES");
		return;
	}

	switch (round(gcode.get('G'))) {
	case 0:
		G00(x, y);
		break;
	case 1:
		G01(x, y);
		break;
	default:
		Serial.println("ERR: UNIMPLEMENTED GCODE");
		return;
	}

	Serial.write("OK: ");
	Serial.println(gcode.m_buf);
	return;
}
