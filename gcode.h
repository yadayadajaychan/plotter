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

#include <ctype.h>
#include <stdlib.h>

class GCode {
public:
	GCode(char* buf, int size) {
		m_buf = buf;
		m_size = size;
		n = 0;
	}

	bool exists(char c) {
		for (int i = 0; i < n; i++) {
			if (m_buf[i] == c)
				return true;
		}
		return false;
	}

	double get(char c) {
		int i = 0;
		while (i < n && m_buf[i] != c)
			i++;

		if (i == n)
			return 0;

		i++;
		//int j = 0;
		//char tmp[8] = {0};
		//while (i < n && j < 7 && (isdigit(m_buf[i]) || m_buf[i] == '.'))
		//	tmp[j++] = m_buf[i++];
		//tmp[j+1] = 0; //null byte

		return atof(m_buf+i);
	}

	bool addChar(char c) {
		if (c == '\n') {
			m_buf[n++] = 0;
			return true;
		}

		if (n < m_size-1)
			m_buf[n++] = c;

		return false;
	}

	void reset() { n = 0; }

	char* m_buf;
	int m_size;
	int n;
};
