﻿#include "pch.h"

#include "vs2crypto.h"

const unsigned char vs2crypto::byte_e[256] = { 83, 205, 74, 50, 251, 77, 51, 154, 78, 48, 201, 72, 211, 26, 134, 107, 7, 158, 250, 93, 64, 122, 140, 248, 133, 225,
193, 111, 120, 18, 31, 247, 210, 167, 237, 109, 186, 57, 14, 168, 4, 6, 121, 88, 123, 164, 178, 156, 54, 113, 0, 34, 165, 17, 58, 215, 97, 80, 172, 66, 139, 234, 75, 
59, 33, 209, 117, 43, 92, 24, 176, 219, 143, 81, 46, 108, 1, 36, 124, 9, 47, 37, 38, 20, 39, 252, 166, 192, 141, 21, 11, 105, 181, 243, 29, 25, 226, 62, 71, 169, 22, 
173, 112, 68, 197, 49, 145, 41, 236, 253, 214, 116, 224, 162, 233, 101, 216, 27, 40, 19, 85, 180, 189, 42, 12, 70, 65, 155, 52, 44, 86, 207, 202, 220, 3, 184, 179, 
245, 69, 223, 106, 128, 221, 146, 61, 188, 149, 232, 10, 182, 63, 198, 53, 95, 132, 119, 199, 90, 16, 160, 239, 255, 249, 203, 185, 55, 131, 5, 159, 110, 177, 227, 
213, 114, 187, 2, 228, 79, 230, 118, 191, 136, 144, 125, 212, 142, 171, 150, 126, 194, 231, 89, 238, 218, 244, 115, 94, 35, 196, 84, 102, 148, 73, 200, 13, 91, 240, 
204, 138, 127, 163, 152, 130, 206, 208, 151, 100, 147, 246, 60, 104, 235, 129, 242, 96, 87, 254, 23, 137, 98, 183, 195, 175, 174, 56, 28, 32, 217, 153, 161, 157, 222, 
30, 45, 99, 103, 229, 82, 8, 135, 15, 241, 67, 170, 190, 76 };

const unsigned char vs2crypto::byte_d[256] = { 50, 76, 175, 134, 40, 167, 41, 16, 248, 79, 148, 90, 124, 204, 38, 250, 158, 53, 29, 119, 83, 89, 100, 227, 69, 95,
13, 117, 235, 94, 242, 30, 236, 64, 51, 197, 77, 81, 82, 84, 118, 107, 123, 67, 129, 243, 74, 80, 9, 105, 3, 6, 128, 152, 48, 165, 234, 37, 54, 63, 219, 144, 97, 150, 
20, 126, 59, 252, 103, 138, 125, 98, 11, 202, 2, 62, 255, 5, 8, 177, 57, 73, 247, 0, 199, 120, 130, 225, 43, 191, 157, 205, 68, 19, 196, 153, 224, 56, 229, 244, 216, 
115, 200, 245, 220, 91, 140, 15, 75, 35, 169, 27, 102, 49, 173, 195, 111, 66, 179, 155, 28, 42, 21, 44, 78, 183, 188, 209, 141, 222, 212, 166, 154, 24, 14, 249, 181, 
228, 208, 60, 22, 88, 185, 72, 182, 106, 143, 217, 201, 146, 187, 215, 211, 238, 7, 127, 47, 240, 17, 168, 159, 239, 113, 210, 45, 52, 86, 33, 39, 99, 253, 186, 58, 
101, 233, 232, 70, 170, 46, 136, 121, 92, 149, 230, 135, 164, 36, 174, 145, 122, 254, 180, 87, 26, 189, 231, 198, 104, 151, 156, 203, 10, 132, 163, 207, 1, 213, 131, 
214, 65, 32, 12, 184, 172, 110, 55, 116, 237, 193, 71, 133, 142, 241, 139, 112, 25, 96, 171, 176, 246, 178, 190, 147, 114, 61, 221, 108, 34, 192, 160, 206, 251, 223, 
93, 194, 137, 218, 31, 23, 162, 18, 4, 85, 109, 226, 161 };

const unsigned char vs2crypto::byte_e2[256] = { 244, 221, 133, 207, 206, 30, 226, 209, 49, 113, 101, 9, 56, 171, 210, 183, 118, 16, 128, 26, 132, 228, 110, 48, 124,
169, 112, 237, 230, 107, 35, 159, 162, 23, 76, 181, 20, 130, 245, 140, 220, 208, 213, 94, 186, 46, 84, 83, 179, 117, 193, 120, 204, 135, 50, 212, 248, 52, 100, 68, 
235, 95, 37, 28, 148, 65, 73, 166, 143, 43, 60, 19, 79, 99, 215, 67, 136, 141, 89, 40, 4, 170, 252, 42, 85, 189, 238, 142, 88, 127, 232, 247, 144, 116, 0, 149, 92, 
32, 29, 187, 5, 253, 234, 200, 214, 70, 175, 154, 96, 126, 102, 211, 62, 152, 185, 119, 71, 205, 160, 36, 239, 125, 243, 59, 134, 131, 33, 45, 81, 168, 191, 222, 74, 
1, 31, 115, 64, 138, 129, 151, 161, 54, 227, 69, 184, 219, 98, 87, 122, 86, 15, 90, 178, 177, 173, 199, 201, 233, 121, 249, 39, 80, 217, 229, 18, 12, 51, 163, 157, 
55, 11, 66, 172, 63, 137, 108, 236, 225, 158, 104, 174, 3, 155, 6, 53, 194, 111, 192, 197, 13, 254, 146, 47, 223, 14, 147, 218, 246, 25, 182, 91, 203, 153, 34, 109, 
145, 97, 77, 82, 150, 224, 231, 202, 8, 75, 61, 93, 190, 196, 250, 139, 2, 57, 24, 164, 114, 58, 41, 105, 198, 165, 156, 38, 21, 44, 103, 251, 240, 241, 255, 195, 
106, 176, 123, 78, 22, 17, 180, 10, 216, 242, 72, 27, 7, 188, 167 };

const unsigned char vs2crypto::byte_d2[256] = { 94, 133, 221, 181, 80, 100, 183, 253, 213, 11, 248, 170, 165, 189, 194, 150, 17, 246, 164, 71, 36, 233, 245, 33, 223,
198, 19, 252, 63, 98, 5, 134, 97, 126, 203, 30, 119, 62, 232, 160, 79, 227, 83, 69, 234, 127, 45, 192, 23, 8, 54, 166, 57, 184, 141, 169, 12, 222, 226, 123, 70, 215, 
112, 173, 136, 65, 171, 75, 59, 143, 105, 116, 251, 66, 132, 214, 34, 207, 244,72, 161, 128, 208, 47, 46, 84, 149, 147, 88, 78, 151, 200, 96, 216, 43, 61,108, 206, 
146, 73, 58, 10, 110, 235, 179, 228, 241, 29, 175, 204, 22, 186, 26, 9, 225, 135, 93, 49, 16, 115, 51, 158, 148, 243, 24, 121, 109, 89, 18, 138, 37,125, 20, 2, 124, 
53, 76, 174, 137, 220, 39, 77, 87, 68, 92, 205, 191,195, 64, 95, 209, 139, 113, 202, 107, 182, 231, 168, 178, 31, 118, 140, 32, 167, 224, 230, 67, 255, 129, 25, 81, 
13, 172, 154, 180, 106, 242, 153, 152, 48, 247, 35, 199, 15, 144, 114, 44, 99, 254, 85, 217, 130, 187, 50, 185, 240, 218, 188, 229, 155, 103, 156, 212, 201, 52, 117, 
4, 3, 41, 7, 14, 111, 55, 42, 104, 74, 249, 162, 196, 145, 40, 1, 131, 193, 210, 177, 6, 142, 21, 163, 28, 211, 90, 157, 102, 60, 176, 27, 86, 120, 237, 238, 250, 
122, 0, 38, 197, 91, 56, 159, 219, 236, 82, 101, 190, 239 };

const unsigned char vs2crypto::byte_s[1024] = { 86, 72, 67, 159, 118, 236, 37, 242, 152, 29, 248, 94, 17, 18, 122, 39, 245, 24, 207, 63, 122, 19, 213, 198, 48, 189,
59, 200, 236, 27, 8, 196, 78, 29, 99, 249, 43, 201, 50, 203, 87, 26, 249, 214, 138, 50, 9, 136, 161, 224, 252, 247, 64, 129, 49, 67, 114, 123, 111, 183, 177, 91, 5, 
124, 21, 52, 16, 243, 173, 128, 250, 44, 228, 4, 231, 95, 24, 254, 60, 252, 179, 177, 38, 11, 29, 15, 131, 204, 234, 31, 135, 175, 90, 121, 25, 250, 21, 125, 192, 86, 
34, 20, 217, 33, 229, 207, 19, 60, 152, 209, 42, 114, 63, 113, 4, 254, 53, 249, 94, 6, 175, 66, 9, 142, 247, 74, 209, 106, 233, 235, 74, 153, 61, 228, 218, 158, 215, 
4, 36, 76, 251, 71, 51, 24, 243, 20, 112, 231, 117, 26, 197, 26, 30, 186, 165, 222, 11, 75, 41, 155, 253, 167, 82, 172, 134, 144, 52, 89, 149, 100, 39, 49, 212, 17, 
9, 104, 15, 239, 153, 226, 217, 232, 244, 207, 38, 48, 123, 130, 63, 250, 147, 139, 217, 136, 78, 83, 243, 134, 145, 99, 223, 23, 82, 16, 241, 147, 69, 12, 176, 61, 
207, 26, 177, 70, 193, 20, 178, 130, 133, 86, 20, 177, 25, 153, 208, 154, 86, 75, 193, 181, 115, 136, 93, 53, 3, 193, 150, 227, 220, 145, 233, 110, 209, 2, 43, 120, 
217, 147, 198, 58, 159, 113, 0, 59, 78, 45, 252, 183, 63, 87, 61, 248, 73, 249, 200, 70, 187, 123, 66, 254, 125, 206, 164, 242, 107, 165, 223, 23, 109, 87, 192, 193, 
132, 135, 22, 1, 135, 34, 229, 212, 196, 221, 164, 133, 84, 239, 36, 214, 56, 227, 188, 93, 80, 198, 188, 207, 204, 12, 207, 168, 77, 228, 74, 230, 150, 123, 116, 193, 
227, 199, 190, 153, 149, 109, 58, 158, 42, 31, 158, 122, 128, 8, 222, 137, 154, 98, 112, 188, 202, 69, 52, 123, 82, 60, 134, 222, 147, 189, 238, 215, 123, 98, 242, 
111, 37, 34, 199, 226, 140, 29, 165, 172, 108, 84, 39, 68, 6, 192, 75, 51, 10, 182, 246, 157, 147, 67, 71, 88, 229, 90, 11, 107, 48, 197, 254, 30, 21, 110, 99, 119, 
154, 141, 48, 172, 159, 134, 14, 252, 21, 65, 216, 210, 113, 186, 205, 195, 107, 85, 219, 225, 218, 110, 251, 49, 18, 57, 123, 165, 84, 8, 47, 150, 92, 211, 151, 124, 
134, 73, 253, 12, 52, 73, 67, 163, 129, 56, 249, 116, 247, 58, 178, 191, 146, 129, 155, 43, 144, 253, 154, 101, 10, 103, 168, 12, 18, 66, 117, 157, 34, 80, 61, 33, 87, 
207, 172, 145, 43, 87, 175, 216, 63, 6, 173, 26, 85, 110, 3, 218, 55, 108, 235, 136, 174, 151, 1, 204, 160, 54, 145, 63, 193, 92, 82, 167, 251, 145, 104, 194, 211, 10, 
0, 57, 77, 192, 37, 3, 185, 141, 182, 162, 62, 154, 101, 181, 100, 180, 114, 190, 205, 223, 240, 243, 197, 111, 23, 226, 179, 5, 29, 195, 6, 64, 7, 45, 107, 178, 58, 
236, 114, 252, 18, 18, 127, 237, 1, 39, 245, 155, 67, 71, 30, 169, 243, 242, 25, 11, 114, 221, 169, 66, 79, 180, 112, 242, 156, 252, 146, 56, 74, 176, 122, 132, 76, 
208, 205, 221, 115, 215, 110, 238, 235, 24, 193, 212, 164, 92, 76, 20, 20, 95, 63, 92, 67, 47, 173, 93, 161, 140, 81, 4, 148, 188, 81, 145, 204, 153, 92, 34, 36, 58, 
150, 154, 110, 185, 86, 54, 242, 154, 77, 63, 175, 126, 36, 233, 113, 29, 65, 28, 34, 112, 60, 230, 133, 106, 90, 46, 198, 47, 8, 11, 254, 42, 238, 216, 199, 163, 236, 
136, 216, 118, 106, 7, 62, 18, 83, 21, 152, 103, 32, 174, 88, 63, 254, 179, 253, 175, 20, 4, 95, 39, 111, 194, 146, 66, 63, 133, 143, 74, 33, 204, 48, 183, 67, 171, 5, 
52, 167, 115, 22, 68, 149, 94, 15, 88, 177, 191, 42, 58, 218, 24, 143, 232, 40, 176, 7, 166, 115, 250, 209, 205, 133, 77, 69, 82, 126, 159, 251, 83, 174, 200, 7, 160, 
236, 26, 144, 16, 179, 57, 143, 127, 91, 84, 111, 9, 61, 71, 142, 145, 164, 93, 191, 172, 160, 71, 190, 71, 25, 187, 217, 18, 194, 213, 139, 17, 16, 237, 250, 13, 31, 
245, 242, 58, 106, 82, 114, 48, 220, 60, 21, 55, 16, 105, 174, 80, 8, 90, 254, 165, 91, 73, 246, 246, 95, 125, 187, 132, 200, 94, 247, 78, 75, 128, 113, 173, 106, 133, 
65, 78, 212, 141, 105, 220, 166, 161, 1, 89, 39, 59, 8, 181, 238, 84, 177, 213, 254, 251, 217, 236, 149, 237, 102, 126, 68, 197, 234, 143, 246, 0, 228, 129, 24, 176, 
208, 204, 222, 26, 79, 189, 124, 98, 92, 141, 85, 65, 95, 245, 206, 154, 174, 110, 171, 84, 183, 229, 39, 234, 151, 223, 182, 113, 97, 39, 85, 133, 156, 61, 131, 227, 
246, 227, 171, 28, 111, 51, 144, 216, 41, 2, 102, 243, 191, 91, 80, 118, 212, 195, 247, 233, 165, 243, 129, 111, 6, 11, 0, 53, 55, 206, 163, 33, 114, 233, 163, 201, 
193, 143, 227, 131, 85, 77, 193, 149, 121, 194, 176, 5, 213, 221, 176, 216, 69, 153, 62, 3, 98, 60, 21, 100, 50, 91, 161, 89, 93, 68, 129, 213, 183, 133, 180, 69, 223, 
115, 148, 88, 50, 235, 244, 67, 89, 249, 94, 88, 30, 202, 219, 215, 171, 160, 103, 133, 121, 126, 80, 248, 171, 103, 55, 52, 65, 217, 77, 80, 41, 106, 247, 208, 5, 
229, 208, 118, 70, 209, 10, 56, 251, 72, 58, 145, 104, 168, 42, 22, 30, 46, 202, 97, 34, 154, 171, 201, 179, 191, 1, 127, 172, 14, 47, 178, 210, 10, 209, 196, 207, 
240, 63, 167, 115, 24, 251, 99, 44, 164, 203, 151 };

void vs2crypto::encrypt(unsigned char * target_data, unsigned char * source_data, long long shift, long long length, unsigned char * key, long long keylen)
{
	if (keylen > 0)
	{

		for (int i = 0; i < length; i++)
		{
			long long sh = shift + i;

			unsigned char c = byte_e[source_data[i]];

			// Fixed part
			c = byte_e2[c];

			// Variable part - key
			c ^= key[(sh % keylen)];

			// Variable part - random table
			c ^= byte_s[(sh % byte_s_len)];

			target_data[i] = c;
		}
	}
}

void vs2crypto::decrypt(unsigned char * target_data, unsigned char * source_data, long long shift, long long length, unsigned char * key, long long keylen)
{
	if (keylen > 0)
	{
		for (int i = 0; i < length; i++)
		{
			long long sh = shift + i;
			unsigned char x = source_data[i];

			// Variable part - random table
			x ^= byte_s[(sh % byte_s_len)];

			// Variable part - key
			x ^= key[(sh % keylen)];

			// Fixed part
			target_data[i] = byte_d2[x];
			target_data[i] = byte_d[target_data[i]];
		}
	}
}