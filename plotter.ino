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

#include <Wire.h>
#include <math.h>
#include "gcode.h"

unsigned char CURR_X = 0;
unsigned char CURR_Y = 0;
// TODO use feed rate instead
float DIST_STEP = 10;
float ANGLE_STEP = 0.01745;
unsigned int FEED_RATE;

unsigned char err = 0;

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

// helper functions
float distance(unsigned char x1, unsigned char y1,
               unsigned char x2, unsigned char y2);
float angle(unsigned char x1, unsigned char y1,
            unsigned char x2, unsigned char y2);

void rotate(unsigned char x, unsigned char y, short i, short j, bool clockwise);

void parse();

void setup()
{
	Wire.begin();
	Wire.setWireTimeout(5000 /*(us)*/, true);
	G00(0, 0);
	Serial.begin(115200);
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
	G00(x, CURR_Y);
}

void setY(unsigned char y)
{
	G00(CURR_X, y);
}

void G00(unsigned char x, unsigned char y)
{
	Wire.beginTransmission(0x28);
	Wire.write(0xa9);
	Wire.write(y); // pot-0
	Wire.write(x); // pot-1
	err = Wire.endTransmission(); //TODO handle errors

	CURR_X = x;
	CURR_Y = y;
}

void G01(unsigned char x, unsigned char y)
{
	float dist = distance(CURR_X, CURR_Y, x, y);
	float ang = angle(CURR_X, CURR_Y, x, y);

	int new_x, new_y;
	float dx, dy;
	while (dist > DIST_STEP) {
		dx = roundf(DIST_STEP * cosf(ang));
		dy = roundf(DIST_STEP * sinf(ang));

		new_x = CURR_X + (int)dx;
		new_y = CURR_Y + (int)dy;

		// check for overflow/underflow
		if (new_x < 0)
			new_x = 0;
		else if (new_x > 255)
			new_x = 255;
		if (new_y < 0)
			new_y = 0;
		else if (new_y > 255)
			new_y = 255;
			
		G00(new_x, new_y);
		dist = distance(CURR_X, CURR_Y, x, y);
		ang = angle(CURR_X, CURR_Y, x, y);
		delay(20); //TODO calculate time delay
	}

	G00(x, y);
	delay(20); //TODO calculate time delay
	return;
}

void G02(unsigned char x, unsigned char y, short i, short j)
{
	rotate(x, y, i, j, true);
}

void G03(unsigned char x, unsigned char y, short i, short j)
{
	rotate(x, y, i, j, false);
}

void rotate(unsigned char x, unsigned char y, short i, short j, bool clockwise)
{
	int rx = CURR_X + i;
	int ry = CURR_Y + j;

	// check for overflow/underflow
	if (rx < 0)
		rx = 0;
	else if (rx > 255)
		rx = 255;
	if (ry < 0)
		ry = 0;
	else if (ry > 255)
		ry = 255;

	float radius = distance(rx, ry, x, y);
	float dist = distance(CURR_X, CURR_Y, x, y);
	float ang = angle(rx, ry, CURR_X, CURR_Y);
	//float end_ang = angle(rx, ry, x, y);

	int new_x, new_y;
	while (dist > DIST_STEP) {
		ang += clockwise ? -ANGLE_STEP : ANGLE_STEP;
		while (ang < 0)
			ang += 2*M_PI;
		while (ang >= 2*M_PI)
			ang -= 2*M_PI;

		new_x = rx + (int)(radius * cosf(ang));
		new_y = ry + (int)(radius * sinf(ang));

		if (new_x < 0)
			new_x = 0;
		else if (new_x > 255)
			new_x = 255;
		if (new_y < 0)
			new_y = 0;
		else if (new_y > 255)
			new_y = 255;

		G00(new_x, new_y);
		dist = distance(CURR_X, CURR_Y, x, y);
		delay(20); //TODO calculate time delay
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

	int g_cmd = round(gcode.get('G'));

	if (!gcode.exists('X') || !gcode.exists('Y')) {
		Serial.println("ERR: MISSING X/Y COORDINATES");
		return;
	}

	int x = round(gcode.get('X'));
	int y = round(gcode.get('Y'));

	if (x < 0 || x > 255 || y < 0 || y > 255) {
		Serial.println("ERR: INVALID X/Y COORDINATES");
		return;
	}

	int i, j;
	if (g_cmd == 2 || g_cmd == 3) {
		if (!gcode.exists('I') || !gcode.exists('J')) {
			Serial.println("ERR: MISSING I/J OFFSETS");
			return;
		}

		i = round(gcode.get('I'));
		j = round(gcode.get('J'));

		int rx = CURR_X + i;
		int ry = CURR_Y + j;

		if (rx < 0 || rx > 255 || ry < 0 || ry > 255) {
			Serial.println("ERR: INVALID I/J OFFSETS");
			return;
		}
	}

	switch (g_cmd) {
	case 0:
		G00(x, y);
		break;
	case 1:
		G01(x, y);
		break;
	case 2:
		G02(x, y, i, j);
		break;
	case 3:
		G03(x, y, i, j);
		break;
	default:
		Serial.println("ERR: UNIMPLEMENTED GCODE");
		return;
	}

	Serial.write("OK: ");
	Serial.println(gcode.m_buf);
	return;
}
