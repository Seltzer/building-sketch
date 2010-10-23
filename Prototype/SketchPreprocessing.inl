#ifndef SKETCH_PREPROCESSING_INL
#define SKETCH_PREPROCESSING_INL




template<int WIDTH, int HEIGHT>
void AddPointToMatrices(bool pixels[WIDTH][HEIGHT], bool pixelsNear[WIDTH][HEIGHT], int fuzzinessThreshold, int2& point)
{
	pixels[point.x][point.y] = true;

	for (int x = -fuzzinessThreshold; x <= fuzzinessThreshold; x++)
	{
		for (int y = -fuzzinessThreshold; y <= fuzzinessThreshold; y++)
		{
			pixelsNear[point.x + x][point.y + y] = true;
		}
	}
}


template<int WIDTH, int HEIGHT>
void StrokesToMatrices(bool pixels[WIDTH][HEIGHT], bool pixelsNear[WIDTH][HEIGHT], int fuzzinessThreshold, std::vector<Stroke>& strokes)
{
	for (int x = 0; x < WIDTH; x++)
	{
		for (int y = 0; y < HEIGHT; y++)
		{
			pixels[x][y] = false;
			pixelsNear[x][y] = false;
		}
	}

	for (std::vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		for (std::vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
			AddPointToMatrices<WIDTH, HEIGHT>(pixels, pixelsNear, fuzzinessThreshold, *it2);
	}
}
















#endif