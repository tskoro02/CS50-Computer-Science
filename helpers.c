#include "helpers.h"

#define HYPO(a, b) sqrt((a) * (a) + (b) * b)

typedef struct Kernel
{
    int x[9];
    int y[9];
}
Kernel;

typedef struct Gvalues
{
    double red;
    double green;
    double blue;
}
Gvalues;

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            double median = ((double) image[i][j].rgbtGreen + image[i][j].rgbtRed + image[i][j].rgbtBlue) / 3;
            median = round(median);
            image[i][j].rgbtGreen = image[i][j].rgbtBlue = image[i][j].rgbtRed = median;
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0, k = width - 1; j < k; j++, k--)
        {
            RGBTRIPLE tmp = image[i][j];
            image[i][j] = image[i][k];
            image[i][k] = tmp;
        }
    }
    return;
}

void blur_convert(int height, int width, RGBTRIPLE tmp[height][width],
                  RGBTRIPLE image[height][width], int i, int j)
{
    int count = 0;
    double red = 0;
    double blue = 0;
    double green = 0;

    for (int k = i - 1; k <= i + 1; k++)
    {
        for (int l = j - 1; l <= j + 1; l++)
        {
            if (k >= 0 && k < height && l >= 0 && l < width)
            {
                red += tmp[k][l].rgbtRed;
                blue += tmp[k][l].rgbtBlue;
                green += tmp[k][l].rgbtGreen;
                count++;
            }
        }
    }

    red /= count;
    blue /= count;
    green /= count;

    image[i][j].rgbtBlue = round(blue);
    image[i][j].rgbtRed = round(red);
    image[i][j].rgbtGreen = round(green);
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE tmp[height][width];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            tmp[i][j] = image[i][j];
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            blur_convert(height, width, tmp, image, i, j);
        }
    }

}

BYTE round_pixel(double value)
{
    if (value > 0xff)
        return 0xff;
    else 
        return round(value);
}

void compute_g(double *gx, double *gy, BYTE pixel_grid, Kernel kernel, int grid)
{
    *gx += pixel_grid * kernel.x[grid];
    *gy += pixel_grid * kernel.y[grid];
}


void compute_edges(int height, int width, RGBTRIPLE tmp[height][width],
                   RGBTRIPLE *pixel, int i, int j)
{
   
    static const Kernel kernel = {
     .x = {-1, 0, 1, -2, 0, 2, -1, 0, 1},
     .y = {-1, -2, -1, 0, 0, 0, 1, 2, 1}
    };

    Gvalues gx = {
        .red = 0.0,
        .green = 0.0,
        .blue = 0.0
    };

    Gvalues gy = {
        .red = 0.0,
        .green = 0.0,
        .blue = 0.0
    };

    int grid_position = 0;

    for (int k = (i - 1); k <= (i + 1); k++)
    {
        for (int l = (j - 1); l <= (j + 1); l++)
        {
            if (k >= 0 && k < height && l >= 0 && l < width)
            {
                compute_g(&gx.red, &gy.red, tmp[k][l].rgbtRed, kernel, grid_position);
                compute_g(&gx.green, &gy.green, tmp[k][l].rgbtGreen, kernel, grid_position);
                compute_g(&gx.blue, &gy.blue, tmp[k][l].rgbtBlue, kernel, grid_position);
            }
            grid_position++;
        }
    }

    Gvalues gt = {
        .red = HYPO(gx.red, gy.red),
        .blue = HYPO(gx.blue, gy.blue),
        .green = HYPO(gx.green, gy.green)
    };


    pixel->rgbtGreen =  round_pixel(gt.green);
    pixel->rgbtBlue =  round_pixel(gt.blue);
    pixel->rgbtRed = round_pixel(gt.red);
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE tmp[height][width];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            tmp[i][j] = image[i][j];
        }
    }
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            compute_edges(height, width, tmp, &image[i][j], i, j);
        }
    }

    return;
}
