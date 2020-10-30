#pragma once
//#define clamp(val, min, max) (val > max ? max : val < min ? min : val)
//#include <GL/glew.h>
//#include <glm/common.hpp>
#include <string>

#define reclass(a_class,a_val) (*(a_class*)&(a_val))
typedef const char* cstring;
typedef  unsigned char ubyte;
typedef  unsigned int uint;
typedef  unsigned short ushort;
//#define unsigned int unsigned int
//#define ushort unsigned short


static cstring cDir(char* dir)
{
	char* tmp;
	if(strlen(dir) > 0)
		while(bool(tmp = strchr(dir, '\\')))
		{
			tmp[0] = '/';
		}

	if(strlen(dir) > 1)
		while(bool(tmp = strstr(dir, "//")))
		{
			memmove_s(tmp, strlen(tmp), tmp + 1, strlen(tmp + 1));
			dir[strlen(dir) - 1] = 0;
		}

	return dir;
}

static std::string tolower(char* dir)
{
	std::string tmp;
	for(unsigned i = 0; i < strlen(dir); i++)
		tmp += (char)tolower(*(dir + i));

	return tmp;
}

template <typename T>
static inline T bezier(const T& p0, const T& t0, const T& t1, const T& p1, const float t)
{
	return
		t * t * t * (-p0 + 3.0f * t0 - 3.0f * t1 + p1) +
		t * t * (3.0f * p0 - 6.0f * t0 + 3.0f * t1) +
		t * (-3.0f * p0 + 3.0f * t0) +
		p0;
}

template <typename T>
static inline T catmull(T p0, T p1, T p2, T p3, float t)
{
	return 0.5f *
		t * t * t * (-p0 + p1 * 3.0f + p2 * -3.0f + p3) +
		t * t * (p0 * 2.0f + p1 * -5.0f + p2 * 4.0f - p3) +
		t * (-p0 + p2) +
		(2.0f * p1);

	//return p0;
}

template<typename T>
static inline T lerp(const T& v0, const T& v1, const float& t)
{
	return (T)((1.0f - t) * v0 + (t)*v1);
}

//?
static inline int vectorWrap(int num, int mod)
{
	return (num + mod) % mod;
}


template<class T = float>
struct Coord2D
{
	union
	{
		struct { T x, y; };
		struct { T u, v; };
		struct { T width, height; };
	};
	/*
	glm::vec2 toVec2()
	{
		return glm::vec2(x, y);
	}
	*/
	Coord2D() = default;

	Coord2D(T a_x, T a_y)
	{
		x = a_x, y = a_y;
	}

	template<class I>
	Coord2D(Coord2D<I> other) { x = (T)other.x; y = (T)other.y; }
	T distance()
	{
		return sqrtf(x * x + y * y);
	}

	static T distance(Coord2D v1, Coord2D v2)
	{
		Coord2D v3 = v2 - v1;
		return sqrtf(v3.x * v3.x + v3.y * v3.y);
	}

	Coord2D normal()
	{
		return *this / distance();
	}

	T& operator[](int m_index)
	{
		T* error = nullptr;
		switch(m_index)
		{
		case 0:
			return static_cast<T&>(x);
		case 1:
			return static_cast<T&>(y);
		}
		return *error;
	}

	Coord2D operator+(Coord2D coord)
	{
		return {x + coord.x, y + coord.y};
	}
	Coord2D operator-(Coord2D coord)
	{
		return {x - coord.x, y - coord.y};
	}

	template<class I>
	friend Coord2D<T> operator*(I scaler, Coord2D<T> coord)
	{
		return {T(scaler * coord.x),T(scaler * coord.y)};
	}

	Coord2D<T> operator*(Coord2D<T> coord)
	{
		return {x * coord.x, y * coord.y};
	}

	template<class I>
	Coord2D<T> operator*(I coord)
	{
		return {T(x * coord), T(y * coord)};
	}

	Coord2D operator/(Coord2D coord)
	{
		return {x / coord.x,y / coord.y};
	}
	template<class I>
	Coord2D operator/(I coord)
	{
		return {T(x / coord),T(y / coord)};
	}
	void operator-=(Coord2D coord)
	{
		x -= coord.x;
		y -= coord.y;
	}
	void operator+=(Coord2D coord)
	{
		x += coord.x;
		y += coord.y;
	}
	void operator/=(Coord2D coord)
	{
		x /= coord.x, y /= coord.y;
	}
	template<class I>
	void operator/=(I coord)
	{
		x /= coord, y /= coord;
	}
};

template<class T = float>
struct Coord3D
{
	union
	{
		struct { T x, y, z; };
		struct { T width, height, depth; };
	};


	Coord3D():x(0), y(0), z(0) {};

	Coord3D(Coord2D<T> coord)
	{
		x = coord.x;
		y = coord.y;
		z = 0;
	}

	void operator=(Coord2D<T> coord)
	{
		x = coord.x;
		y = coord.y;
	}

	Coord3D(Coord2D<T> coord, T last)
	{
		x = coord.x;
		y = coord.y;
		z = last;
	}

	Coord3D(T scale)
	{
		this->x = scale;
		this->y = scale;
		this->z = scale;
	}

	Coord3D(T m_x, T m_y, T m_z)
	{
		this->x = m_x;
		this->y = m_y;
		this->z = m_z;
	}

	Coord3D(T m_x, T m_y)
	{
		this->x = m_x;
		this->y = m_y;
		this->z = 0;
	}

	/*
	glm::vec3 toVec3()
	{
		return {x,y,z};
	}

	static glm::vec3 toVec3(Coord3D<float> a0)
	{
		return reclass(glm::vec3, a0);
	}

	static glm::vec3 toVec3(Coord3D<int> a0)
	{
		return reclass(glm::vec3, a0);
	}
	*/

	static T distance(Coord3D<T> v1, Coord3D<T> v2)
	{
		v1 -= v2;
		return v1.distance();
	}

	T distance()
	{
		return sqrtf(x * x + y * y + z * z);
	}

	T distanceSquare()
	{
		return (x * x + y * y + z * z);
	}

	static T dotProduct(Coord3D<T> a, Coord3D<T> b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Coord3D<T> crossProduct(Coord3D<T> a, Coord3D<T> b)
	{
		return
		{
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}
	friend static Coord3D<T> abs(Coord3D<T> val)
	{
		return {sqrtf(val.x * val.x),sqrtf(val.y * val.y),sqrtf(val.z * val.z)};
	}


	Coord3D<T> normal()
	{
		return *this / distance();
	}


	void set(Coord2D<T> coord)
	{
		x = coord.x;
		y = coord.y;
	}

	void set(T m_x, T m_y, T m_z)
	{
		this->x = m_x;
		this->y = m_y;
		this->z = m_z;
	}

	void set(T m_x, T m_y)
	{
		this->x = m_x;
		this->y = m_y;
	}

	void normalize()
	{
		T norm = sqrtf(x * x + y * y + z * z);
		x /= norm;
		y /= norm;
		z /= norm;
	}

	T& operator[] (int m_index)
	{
		T* error = nullptr;
		switch(m_index)
		{
		case 0:
			return const_cast<T&>(x);
		case 1:
			return const_cast<T&>(y);
		case 2:
			return const_cast<T&>(z);
		}
		return *error;
	}

	Coord3D<T> operator+(Coord3D<T> coord)
	{
		return {T(x + coord.x), T(y + coord.y), T(z + coord.z)};
	}

	Coord3D<T> operator-(Coord3D<T> coord)
	{
		return {T(x - coord.x), T(y - coord.y), T(z - coord.z)};
	}

	friend Coord3D<T> operator-(T val, Coord3D<T> coord)
	{
		return {T(val - coord.x), T(val - coord.y), T(val - coord.z)};
	}

	friend Coord3D<T> operator*(T scaler, Coord3D<T> coord)
	{
		return {scaler * coord.x, scaler * coord.y, scaler * coord.z};
	}

	Coord3D<T> operator*(Coord3D<T> coord)
	{
		return {x * coord.x, y * coord.y, z * coord.z};
	}

	Coord3D<T> operator*(T coord)
	{
		return {x * coord, y * coord, z * coord};
	}

	Coord3D<T> operator/(Coord3D<T> coord)
	{
		return {x / coord.x,y / coord.y,z / coord.z};
	}

	Coord3D<T> operator/(T coord)
	{
		return {x / coord,y / coord,z / coord};
	}

	Coord3D<T>& operator-()
	{
		static Coord3D<T> tmp;
		tmp = *this * -1;
		return tmp;
	}

	void operator-=(Coord3D<T> coord)
	{
		x -= coord.x;
		y -= coord.y;
		z -= coord.z;
	}

	void operator+=(Coord3D<T> coord)
	{
		x += coord.x;
		y += coord.y;
		z += coord.z;
	}

	void operator*=(Coord3D<T> coord)
	{
		x *= coord.x;
		y *= coord.y;
		z *= coord.z;
	}

	void operator*=(T val)
	{
		x *= val;
		y *= val;
		z *= val;
	}

	void operator/=(T val)
	{
		x /= val;
		y /= val;
		z /= val;
	}


	bool operator==(Coord3D<T> coord)const
	{
		return
			x == coord.x &&
			y == coord.y &&
			z == coord.z;
	}

	bool operator!=(Coord3D<T> coord)const
	{
		return !(*this == coord);
	}

	bool operator>(Coord3D<T> coord)
	{
		return this->distanceSquare() > coord.distanceSquare();
	}

	bool operator<=(Coord3D<T> coord)
	{
		return !(*this > coord);
	}
	bool operator<(Coord3D<T> coord)
	{
		return this->distanceSquare() < coord.distanceSquare();
	}
	bool operator>=(Coord3D<T> coord)
	{
		return !(*this < coord);
	}
};

struct ColourRGBA
{
	ubyte r, g, b, a;

	ColourRGBA():r((ubyte)255), g((ubyte)255), b((ubyte)255), a((ubyte)255)
	{}

	ColourRGBA(ubyte a_r, ubyte a_g, ubyte a_b, ubyte a_a = (ubyte)255):r(a_r), g(a_g), b(a_b), a(a_a)
	{}

	void set(ColourRGBA rgba)
	{
		set(
			rgba.r,
			rgba.g,
			rgba.b,
			rgba.a);
	}

	void set(ubyte a_r, ubyte a_g, ubyte a_b)
	{
		this[0][0] = a_r;
		this[0][1] = a_g;
		this[0][2] = a_b;
	}

	void set(ubyte a_r, ubyte a_g, ubyte a_b, ubyte a_a)
	{
		this[0][0] = a_r;
		this[0][1] = a_g;
		this[0][2] = a_b;
		this[0][3] = a_a;
	}

	void set(float a_r, float a_g, float a_b)
	{
		this[0][0] = ubyte(a_r * 255);
		this[0][1] = ubyte(a_g * 255);
		this[0][2] = ubyte(a_b * 255);
	}

	void set(float a_r, float a_g, float a_b, float a_a)
	{
		this[0][0] = ubyte(a_r * 255);
		this[0][1] = ubyte(a_g * 255);
		this[0][2] = ubyte(a_b * 255);
		this[0][3] = ubyte(a_a * 255);
	}

	ColourRGBA operator+(ColourRGBA rgba)
	{
		return ColourRGBA{
			ubyte(r + rgba[0]),
			ubyte(g + rgba[1]),
			ubyte(b + rgba[2]),
			ubyte(a + rgba[3])};
	}

	ColourRGBA operator*(float rgba)
	{
		return ColourRGBA{
			ubyte(r * rgba),
			ubyte(g * rgba),
			ubyte(b * rgba),
			ubyte(a * rgba)};
	}

	ColourRGBA operator/(float rgba)
	{
		return ColourRGBA{
			ubyte(r / rgba),
			ubyte(g / rgba),
			ubyte(b / rgba),
			ubyte(a / rgba)};
	}

	friend ColourRGBA operator*(float rgba, ColourRGBA colour)
	{
		return ColourRGBA{
			ubyte(colour[0] * rgba),
			ubyte(colour[1] * rgba),
			ubyte(colour[2] * rgba),
			ubyte(colour[3] * rgba)};
	}
	friend ColourRGBA operator/(float rgba, ColourRGBA colour)
	{
		return ColourRGBA{
			ubyte(colour[0] / rgba),
			ubyte(colour[1] / rgba),
			ubyte(colour[2] / rgba),
			ubyte(colour[3] / rgba)};
	}
	ColourRGBA operator*(ColourRGBA rgba)
	{
		return ColourRGBA{
			ubyte(r * rgba.r / 255.f),
			ubyte(g * rgba.g / 255.f),
			ubyte(b * rgba.b / 255.f),
			ubyte(a * rgba.a / 255.f)};
	}

	ColourRGBA operator/(ColourRGBA rgba)
	{
		return ColourRGBA{
			ubyte(r / (float)rgba.r / 255.f),
			ubyte(g / (float)rgba.g / 255.f),
			ubyte(b / (float)rgba.b / 255.f),
			ubyte(a / (float)rgba.a / 255.f)};
	}

	void operator*=(ColourRGBA rgba)
	{
		*this = *this * rgba;
	}

	void operator/=(ColourRGBA rgba)
	{
		*this = *this / rgba;
	}

	ubyte& operator[](int index)
	{
		ubyte* error = nullptr;
		switch(index)
		{
		case 0:
			return static_cast<ubyte&>(r);
		case 1:
			return static_cast<ubyte&>(g);
		case 2:
			return static_cast<ubyte&>(b);
		case 3:
			return static_cast<ubyte&>(a);
		}
		return *error;
	}
};

struct UV
{
	float u = 0, v = 0;

	void set(float a_u, float a_v)
	{
		this->u = a_u;
		this->v = a_v;
	}

	float& operator[](int m_index)
	{
		switch(m_index)
		{
		case 0:
			return static_cast<float&>(u);
		case 1:
			return static_cast<float&>(v);
		}
		float* error = nullptr;
		return *error;
	}

	bool operator==(UV uv)
	{
		return u == uv.u && v == uv.v;
	}
};

struct Vertex2D
{
	Coord2D<> coord;
	ColourRGBA	colour;
	UV uv;

	void setCoord2D(float x, float y)
	{
		coord.x = x;
		coord.y = y;
	}
	//uses the 0-255 representation instead of 0-1
	void setColour(ubyte r, ubyte g, ubyte b, ubyte a = 255)
	{
		colour.r = r;
		colour.g = g;
		colour.b = b;
		colour.a = a;
	}
	//sets uv
	void setUV(float u, float v)
	{
		uv.u = u;
		uv.v = v;
	}

	void print()
	{
		printf("Coord2D: (%f, %f)\n", coord.x, coord.y);
		printf("Colour : (%d, %d, %d, %d)\n", colour.r, colour.g, colour.b, colour.a);
		printf("UV     : (%f, %f)\n\n", uv.u, uv.v);
	}
};

struct Vertex3D
{
	Coord3D<float> coord, norm;
	ColourRGBA	colour;
	UV uv;

	void setCoord(float x, float y, float z)
	{
		coord.x = x;
		coord.y = y;
		coord.z = z;
	}
	//uses the 0-255 representation instead of 0-1
	void setColour(ubyte r, ubyte g, ubyte b, ubyte a = 255)
	{
		colour.r = r;
		colour.g = g;
		colour.b = b;
		colour.a = a;
	}
	//sets uv
	void setUV(float u, float v)
	{
		uv.u = u;
		uv.v = v;
	}

	void setNorm(float x, float y, float z)
	{
		norm.x = x;
		norm.y = y;
		norm.z = z;
	}

	bool operator==(Vertex3D vert)
	{
		return coord == vert.coord && norm == vert.norm && uv == vert.uv;
	}

	void print()
	{
		printf("Coord3D: (%f, %f, %f)\n", coord.x, coord.y, coord.z);
		printf("Colour : (%d, %d, %d, %d)\n", colour.r, colour.g, colour.b, colour.a);
		printf("UV     : (%f, %f)\n\n", uv.u, uv.v);
	}
};

struct Indicie
{

	unsigned coord = 0;
	unsigned uv = 0;
	unsigned norm = 0;

	unsigned& operator[](int m_index)
	{
		switch(m_index)
		{
		case 0:
			return static_cast<unsigned&>(coord);
		case 1:
			return static_cast<unsigned&>(uv);
		case 2:
			return static_cast<unsigned&>(norm);
		}
		unsigned* error = nullptr;
		return *error;
	}

	friend bool operator<(const Indicie& in1, const Indicie& in2)
	{
		return in1.coord < in2.coord;
	}

	bool operator==(Indicie in)
	{
		for(short i = 0; i < 3; ++i)
			if((*this)[i] != in[i])
				return false;
		return true;

		//return (*this)[0] != in[0];
	}

	void correct()
	{
		coord -= 1;
		norm -= 1;
		uv -= 1;
	}
};

struct WindowInfo
{
	std::string title;
	Coord3D<int> position, size;
	int monitor;
	void print()
	{
		printf("Title    : %s\n\n", title.c_str());
		printf("Position : (%d, %d)\n", position.x, position.y);
		printf("Size     : (%d, %d, %d)\n", size.width, size.height, size.depth);
		printf("Monitor  : %d\n\n", monitor);
	}
};