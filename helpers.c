#include "helpers.h"

#define Y (i + k)
#define X (j + l)


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

    for (int k = -1; k <= 1; k++)
    {
        for (int l = -1; l <= 1; l++)
        {
            if (Y >= 0 && Y < height && X >= 0 && X < width)
            {
                red += tmp[Y][X].rgbtRed;
                blue += tmp[Y][X].rgbtBlue;
                green += tmp[Y][X].rgbtGreen;
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

void round_up(double *value)
{
    if (*value > 0xff)
        *value = 0xff;
    *value = round(*value);
    return;
}

void compute_edges(int height, int width, RGBTRIPLE tmp[height][width],
                   RGBTRIPLE image[height][width], int i, int j)
{
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

    for (int k = -1; k <= 1; k++)
    {
        for (int l = -1; l <= 1; l++)
        {
            if (Y >= 0 && Y < height && X >= 0 && X < width)
            {
                gx.red += tmp[Y][X].rgbtRed * kernel.x[grid_position];
                gx.blue += tmp[Y][X].rgbtBlue * kernel.x[grid_position];
                gx.green += tmp[Y][X].rgbtGreen * kernel.x[grid_position];
                gy.red += tmp[Y][X].rgbtRed * kernel.y[grid_position];
                gy.blue += tmp[Y][X].rgbtBlue * kernel.y[grid_position];
                gy.green += tmp[Y][X].rgbtGreen * kernel.y[grid_position];
            }
            grid_position++;
        }
    }

    Gvalues gt = {
        .red = sqrt(gx.red * gx.red + gy.red * gy.red),
        .blue = sqrt(gx.blue * gx.blue + gy.blue * gy.blue),
        .green = sqrt(gx.green * gx.green + gy.green * gy.green),
    };

    round_up(&gt.red);
    round_up(&gt.green);
    round_up(&gt.blue);

    image[i][j].rgbtGreen =  gt.green;
    image[i][j].rgbtBlue =  gt.blue;
    image[i][j].rgbtRed = gt.red;
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
            compute_edges(height, width, tmp, image, i, j);
        }
    }

    return;
}
