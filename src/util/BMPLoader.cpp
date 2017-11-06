#include "BMPLoader.h"

int BmpLoader::loadBmp(const char * FilePath, IMAGEDATA * ImageData)
{
	char debugStringBuf[256];
	sprintf(debugStringBuf, "ExampleGuage::BitmapLoader - FilePath: %s\n", FilePath);
	XPLMDebugString(debugStringBuf);
	BMPFILEHEADER   Header;
	BMPINFOHEADER	ImageInfo;
	int						Padding;
	FILE *					BitmapFile = NULL;
	int RetCode = 0;

	ImageData->pData = NULL;

	BitmapFile = fopen(FilePath, "rb");
	if (BitmapFile != NULL)
	{
		if (fread(&Header, sizeof(Header), 1, BitmapFile) == 1)
		{
			if (fread(&ImageInfo, sizeof(ImageInfo), 1, BitmapFile) == 1)
			{
				/// Handle Header endian.
				SwapEndian(&Header.bfSize);
				SwapEndian(&Header.bfOffBits);

				/// Handle ImageInfo endian.
				SwapEndian(&ImageInfo.biWidth);
				SwapEndian(&ImageInfo.biHeight);
				SwapEndian(&ImageInfo.biBitCount);

				short channels = ImageInfo.biBitCount / 8;

				/// Make sure that it is a bitmap.
#if APL && defined(__POWERPC__)
				if (((Header.bfType & 0xff) == 'M') &&
					(((Header.bfType >> 8) & 0xff) == 'B') &&
#else
				if (((Header.bfType & 0xff) == 'B') &&
					(((Header.bfType >> 8) & 0xff) == 'M') &&
#endif
					(ImageInfo.biBitCount == 24 || ImageInfo.biBitCount == 32) &&
					(ImageInfo.biWidth > 0) &&
					(ImageInfo.biHeight > 0))
				{
					/// "Header.bfSize" does not always agree
					/// with the actual file size and can sometimes be "ImageInfo.biSize"	 smaller.
					/// So add it in for good measure
					if ((Header.bfSize + ImageInfo.biSize - Header.bfOffBits) >= (ImageInfo.biWidth * ImageInfo.biHeight * channels))
					{
						Padding = (ImageInfo.biWidth * channels + channels) & ~channels;
						Padding -= ImageInfo.biWidth * channels;

						ImageData->width = ImageInfo.biWidth;
						ImageData->height = ImageInfo.biHeight;
						ImageData->Padding = Padding;

						/// Allocate memory for the actual image.
						ImageData->channels = channels;
						ImageData->pData = (unsigned char *)malloc(ImageInfo.biWidth * ImageInfo.biHeight * ImageData->channels + ImageInfo.biHeight * Padding);

						if (ImageData->pData != NULL)
						{
							/// Get the actual image.
							if (fread(ImageData->pData, ImageInfo.biWidth * ImageInfo.biHeight * ImageData->channels + ImageInfo.biHeight * Padding, 1, BitmapFile) == 1)
							{
								RetCode = 1;
							}
							else {
								XPLMDebugString("Failed to load bitmap - failed to read image data\n");
							}
						}
						else {
							XPLMDebugString("Failed to load bitmap - ImageData->pdata was null\n");
						}
					}
					else {
						XPLMDebugString("Failed to load bitmap - header.bfSize + ...\n");
					}
				}
				else {
					XPLMDebugString("Failed to load bitmap - header is not declared as bitmap\n");
				}
			}
			else {
				XPLMDebugString("Failed to read bitmap info.\n");
			}
		}
		else {
			XPLMDebugString("Failed to read bitmap header\n");
		}
	}
	else {
		XPLMDebugString("Bitmap file was null\n");
	}
	if (BitmapFile != NULL)
		fclose(BitmapFile);
	return RetCode;
}
/// Swap the red and blue pixels.
void BmpLoader::swapRedBlue(IMAGEDATA *ImageData)
{
	unsigned char  * srcPixel;
	int		x, y;
	unsigned char sTemp;
	short channels = ImageData->channels;

	/// Do the swap
	srcPixel = ImageData->pData;

	for (y = 0; y < ImageData->height; ++y) {
		for (x = 0; x < ImageData->width; ++x)
		{
			sTemp = srcPixel[0];
			srcPixel[0] = srcPixel[2];
			srcPixel[2] = sTemp;

			srcPixel += channels;
			if (x == (ImageData->width - 1))
				srcPixel += ImageData->Padding;
		}
	}
}