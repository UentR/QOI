#include <fstream>
#include <iostream>
#include <bitset>
#include <algorithm>

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

int endian_swap(int x) {
    return (x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24);
}

int Idx(int r, int g, int b, int a=0) {
    return (r*3 + g*5 + b*7 + a*11) % 64;
}

int Idx(PIXEL p) {
    return Idx(p.r, p.g, p.b, p.a);
}

int main() {
    ifstream file("ImageTest/kodim10.qoi", ios::binary | ios::in);
    
    if (!file.is_open()) {
        std::cout << "Error: File not found" << endl;
        return 0;
    }

    char c;
    file.read(&c, 4); // QOI

    unsigned int width, height;
    file.read((char*)&width, 4);
    width = endian_swap(width);
    file.read((char*)&height, 4);
    height = endian_swap(height);
    std::cout << width << endl;
    std::cout << height << endl;

    unsigned int channels, colorspace;
    file.read((char*)&channels, 1);
    file.read((char*)&colorspace, 1);
    std::cout << channels << endl;
    std::cout << colorspace << endl;

    int size = width * height;
    PIXEL *data = new PIXEL[size];

    PIXEL *seen = new PIXEL[64]();

    unsigned char dataChunk;
    int Continue = 0;
    PIXEL LastPixel{0, 0, 0, 255};
    PIXEL DeltaPixel{0, 0, 0, 0};
    int flag;
    unsigned char r, g, b, a;
    char dr, dg, db;
    char diffGreen, drg, dbg;
    int idx, run;
    int i=0;
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

        if ((int)dataChunk == 0b11111111) {
            file.read((char*)&r, 1);
            file.read((char*)&g, 1);
            file.read((char*)&b, 1);
            file.read((char*)&a, 1);
            data[i] = PIXEL(r, g, b, a);
        } else if ((int)dataChunk == 0b11111110) {
            file.read((char*)&r, 1);
            file.read((char*)&g, 1);
            file.read((char*)&b, 1);
            data[i] = PIXEL(r, g, b, LastPixel.a);
        } else {
            flag = dataChunk >> 6;
            if (flag == 0b00) {
                idx = dataChunk & 0b00111111;
                data[i] = seen[idx];
            } else if (flag == 0b01) {
                dr = ((dataChunk && 0b00110000) >> 4) - 2;
                dg = ((dataChunk && 0b00001100) >> 2) - 2;
                db = (dataChunk &&  0b00000011) - 2;
                DeltaPixel = PIXEL(dr, dg, db, 0);
                data[i] = DeltaPixel+LastPixel;
            } else if (flag == 0b10) {
                diffGreen = (dataChunk & 0b00111111) - 32;
                file.read((char*)&dataChunk, 1);
                drg = (dataChunk >> 4) - 8;
                dbg = (dataChunk & 0b00001111) - 8;
                dr = drg + diffGreen;
                db = dbg + diffGreen;
                DeltaPixel = PIXEL(dr, diffGreen, db, 0);
                data[i] = DeltaPixel+LastPixel;
            } else if (flag == 0b11) {
                run = (dataChunk & 0b00111111) + 1;
                i += run-1;
                fill_n(data+i, run, LastPixel);
            }
        }

        LastPixel = data[i];
        idx = Idx(LastPixel);
        seen[idx] = LastPixel;
        i++;
    }
    file.close();

    ofstream out("ImageTest/kodim10.ppm", ios::binary | ios::out);
    out << "P6" << endl;
    out << width << " " << height << endl;
    out << "255" << endl;

    for (int i=0; i<size; i++) {
        out << data[i].r << data[i].g << data[i].b;
    }

    return 1;
}