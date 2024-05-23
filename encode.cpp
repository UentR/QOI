#include <fstream>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <string>

using namespace std;

struct PIXEL {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    PIXEL(): r(0), g(0), b(0), a(0) {}
    PIXEL(unsigned char r, unsigned char g, unsigned char b, unsigned char a): r(r), g(g), b(b), a(a) {}
    PIXEL(unsigned char r, unsigned char g, unsigned char b): r(r), g(g), b(b), a(255) {}
};

ostream& operator<<(ostream& os, const PIXEL& p) {
    os << "R: " << (int)p.r << " G: " << (int)p.g << " B: " << (int)p.b << " A: " << (int)p.a;
    return os;
}

PIXEL operator+(const PIXEL& p1, const PIXEL& p2) {
    int r = p1.r + p2.r;
    int g = p1.g + p2.g;
    int b = p1.b + p2.b;
    int a = p1.a + p2.a;

    if (r > 255) r -= 256;
    if (g > 255) g -= 256;
    if (b > 255) b -= 256;
    if (a > 255) a -= 256;
    if (r < 0) r += 256;
    if (g < 0) g += 256;
    if (b < 0) b += 256;
    if (a < 0) a += 256;
    return PIXEL(r, g, b, a);
}

bool operator==(const PIXEL& p1, const PIXEL& p2) {
    return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b && p1.a == p2.a;
}

unsigned int endian_swap(unsigned int x) {
    return (x>>24) | ((x>>8) & 0x0000FF00) | ((x<<8) & 0x00FF0000) | (x<<24);
}

int Idx(int r, int g, int b, int a=0) {
    return (r*3 + g*5 + b*7 + a*11) % 64;
}

int Idx(PIXEL p) {
    return Idx(p.r, p.g, p.b, p.a);
}

int main() {
    string fileName = "wikipedia_008";
    ifstream file("ImageTest/" + fileName + ".ppm", ios::binary | ios::in);
    
    if (!file.is_open()) {
        std::cout << "Error: File not found" << endl;
        return 0;
    }
    
    string PPM;
    int width, height, maxColor;
    file >> PPM >> width >> height >> maxColor;

    if (maxColor != 255) {
        std::cout << "Error: Max color is not 255" << endl;
        return 0;
    }

    int size = width * height;
    PIXEL *data = new PIXEL[size];
    unsigned char r, g, b;
    int i = 0;
    while (file >> r >> g >> b) {
        data[i] = PIXEL(r, g, b);
        i++;
    }
    file.close();

    ofstream out("ImageTest/"+fileName+"New.qoi", ios::binary | ios::out);
    out.write("qoif", 4);

    unsigned int widthSwap = endian_swap(width);
    unsigned int heightSwap = endian_swap(height);
    out.write((char*)&widthSwap, 4);
    out.write((char*)&heightSwap, 4);
    char channels = 0b11, colorspace = 0b0;
    out.write((char*)&channels, 1);
    out.write((char*)&colorspace, 1);

    
    PIXEL *seen = new PIXEL[64]();
    PIXEL LastPixel{0, 0, 0, 255};
    PIXEL DeltaPixel{0, 0, 0, 0};
    PIXEL Current;
    unsigned char dataChunk;
    unsigned char run;
    for (i=0; i<size; i++) {
        Current = data[i];
        unsigned char idx = Idx(Current);
        if (Current == LastPixel) {
            run = 0;
            while (data[i] == LastPixel && run < 62) {
                run++;
                i++;
            }
            i--;
            dataChunk = run | 0b11000000;
            out.write((char*)&dataChunk, 1);
            continue;
        }
        if (seen[idx] == Current) {
            dataChunk = idx;
            out.write((char*)&dataChunk, 1);
            continue;
        }



        seen[idx] = Current;
    }

    int flag;
    unsigned char a;
    char dr, dg, db;
    char diffGreen, drg, dbg;
    unsigned char idx, run;
    i=0;
    while (Continue<8) { 
        file.read((char*)&dataChunk, 1);
        
        if (dataChunk == 0) {
            Continue++;
        } else if (dataChunk == 1) {
            if (Continue > 0) {
                Continue++;
            } else {
                Continue = 0;
            }
        } else {
            Continue = 0;
        }

        if ((int)dataChunk == 0b11111111) { // Works
            file.read((char*)&r, 1);
            file.read((char*)&g, 1);
            file.read((char*)&b, 1);
            file.read((char*)&a, 1);
            data[i] = PIXEL(r, g, b, a);
        } else if ((int)dataChunk == 0b11111110) { // Works
            file.read((char*)&r, 1);
            file.read((char*)&g, 1);
            file.read((char*)&b, 1);
            data[i] = PIXEL(r, g, b, LastPixel.a);
        } else {
            flag = dataChunk >> 6;
            if (flag == 0b00) { // Works
                idx = dataChunk & 0b00111111;
                data[i] = seen[idx];
            } else if (flag == 0b01) { // Works
                dr = ((dataChunk & 0b00110000) >> 4) - 2;
                dg = ((dataChunk & 0b00001100) >> 2) - 2;
                db = (dataChunk &  0b00000011) - 2;
                PIXEL Delta(dr, dg, db, 0);
                data[i] = Delta+LastPixel;
            } else if (flag == 0b10) { // Works
                diffGreen = (dataChunk & 0b00111111) - 32;
                file.read((char*)&dataChunk, 1);
                drg = (dataChunk >> 4) - 8;
                dbg = (dataChunk & 0b00001111) - 8;
                dr = drg + diffGreen;
                db = dbg + diffGreen;
                DeltaPixel = PIXEL(dr, diffGreen, db, 0);
                data[i] = DeltaPixel+LastPixel;
            } else if (flag == 0b11) { // Works
                run = (dataChunk & 0b00111111) + 1;
                std::fill_n(data+i, run, LastPixel);
                i += run-1;
            }
        }

        LastPixel = data[i];
        idx = Idx(LastPixel);
        seen[idx] = LastPixel;
        i++;
    }
    
    return 1;
}