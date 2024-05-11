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
