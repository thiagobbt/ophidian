#include <iostream>
#include <algorithm>

#include <array>

template<int width, typename type>
class LUT2D {
public:
	using Line = std::array<type, width>;
	using Table = std::array<Line, width>;

private:
	const Line indexX;
	const Line indexY;
	const Table table;

public:
	LUT2D(const Line& x, const Line& y, const Table& t) : indexX(x), indexY(y), table(t) {}

	type lookup(type y, type x) {
		auto lowerX = std::lower_bound(indexX.begin(), indexX.end(), x);
		auto lowerY = std::lower_bound(indexY.begin(), indexY.end(), y);

		type x2 = *lowerX;
		type y2 = *lowerY;
		type x1 = *(--lowerX);
		type y1 = *(--lowerY);

		type q11 = table[y1][x1];
		type q12 = table[y2][x1];
		type q21 = table[y1][x2];
		type q22 = table[y2][x2];

		type weight0 = ((x2 - x)/(x2 - x1));
		type weight1 = ((x - x1)/(x2 - x1));
		type weight2 = ((y2 - y)/(y2 - y1));
		type weight3 = ((y - y1)/(y2 - y1));

		type r1 = weight0*q11 + weight1*q21;
		type r2 = weight0*q12 + weight1*q22;
		type p  = weight2*r1  + weight3*r2;

		return p;
	}

	Line lookupLine(Line y, Line x) {
		std::array<const float*, width> lowerX, lowerY;
		Line x1, x2, y1, y2;
		Line weight0, weight1, weight2, weight3;
		Line q11, q12, q21, q22;
		Line r1, r2, p;
		
		// Multiple fors with the purpose of trying to use SSE or AVX instructions

		for (int i = 0; i < width; i++) {
			lowerX[i] = std::lower_bound(indexX.begin(), indexX.end(), x[i]);
			lowerY[i] = std::lower_bound(indexY.begin(), indexY.end(), y[i]);
		}

		for (int i = 0; i < width; i++) {
			x2[i] = *lowerX[i];
			y2[i] = *lowerY[i];
			x1[i] = *(--lowerX[i]);
			y1[i] = *(--lowerY[i]);
		}

		for (int i = 0; i < width; i++) {
			weight0[i] = ((x2[i] - x[i])/(x2[i] - x1[i]));
			weight1[i] = ((x[i] - x1[i])/(x2[i] - x1[i]));
			weight2[i] = ((y2[i] - y[i])/(y2[i] - y1[i]));
			weight3[i] = ((y[i] - y1[i])/(y2[i] - y1[i]));
		}

		for (int i = 0; i < width; i++) {
			q11[i] = table[y1[i]][x1[i]];
			q12[i] = table[y2[i]][x1[i]];
			q21[i] = table[y1[i]][x2[i]];
			q22[i] = table[y2[i]][x2[i]];
		}

		for (int i = 0; i < width; i++) {
			r1[i] = weight0[i] * q11[i] + weight1[i] * q21[i];
			r2[i] = weight0[i] * q12[i] + weight1[i] * q22[i];
			 p[i] = weight2[i] *  r1[i] + weight3[i] *  r2[i];
		}

		return p;
	}
};

float count = 0;
float gen() {
	count += 0.5;
	return count;
}

int main(int argc, char const *argv[]) {
	using LUT = LUT2D<8, float>;

	LUT::Line x = {{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f}};
	LUT::Line y = {{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f}};
	//LUT::Table table = {{0.0}};
	LUT::Table table;

	for (auto it = table.begin(); it < table.end(); it++) {
		std::generate((*it).begin(), (*it).end(), gen);
	}

	//LUT t = LUT(x, y, {{0.0f}});
	LUT t = LUT(x, y, table);

	float xip;
	float yip;

	/*while (true) {
		std::cout << "> ";
		std::cin >> yip >> xip;
		std::cout << "lookup(" << yip << ", " << xip << ") = " << t.lookup(yip, xip) << std::endl;
	}*/

	while (true) {
		LUT::Line ptsX;
		LUT::Line ptsY;
		for (int i = 0; i < 8; ++i) {
			std::cout << "Enter point " << i << ": ";
			std::cin >> ptsY[i] >> ptsX[i];
		}
		auto results = t.lookupLine(ptsY, ptsX);

		for (int i = 0; i < 8; i++) {
			std::cout << "lookup(" << ptsY[i] << ", " << ptsX[i] << ") = " << results[i] << std::endl;
		}
	}

	return 0;
}
