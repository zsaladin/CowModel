#ifndef OPENOBJ
#define OPENOBJ

#include <vector>
#include <ctype.h>


struct pos3d {
	float x, y, z;
};



#define	BUFFER_SIZE	20000							// length of longest input line in ".obj" file (including continuations)
#define	FACE_SIZE	4096							// maximum number of vertices in a single polygon
#define	MAXSTRING	1024							// string buffer size
#define	SAME(_a, _b)	(strcmp(_a,_b) == 0)		// case sensitive string equality test

bool openObj(char *filename, std::vector<pos3d> &varray, std::vector<std::vector<int> > &polyarray)
{
	FILE		*objFile;

	char		buffer[BUFFER_SIZE];
	char		token[BUFFER_SIZE];
	char		*next = NULL;
	char		*backslash = NULL;
	char		before_token[BUFFER_SIZE];
	int			width, nVertex, ntVertex, nPolygonCounts, nPolygonConnects;
	
	width = nVertex = ntVertex = nPolygonCounts = nPolygonConnects = 0;

	pos3d p;

	varray.clear();
	polyarray.clear();
	
	int vtxIndex = 0;
	int faceIndex = 0;
	
	if ((objFile = fopen(filename, "r")) == NULL)
		return false;

	while (fgets(buffer, BUFFER_SIZE, objFile) != NULL) 
	{
		while ((backslash = strchr(buffer, '\\')) != NULL)
			if (fgets(backslash,
			    (int) (BUFFER_SIZE - strlen(buffer)),
			    objFile) == NULL)
				break;

		for (next = buffer; *next != '\0' && isspace(*next); next++)
			;	// EMPTY 

		if (*next == '\0' || *next == '#' ||
		    *next == '!' || *next == '$')
			continue;

		sscanf(next, "%s%n", token, &width);
		next += width;

		if (SAME(token, "v")) {
			sscanf(next, "%f%f%f", &p.x, &p.y, &p.z);
			varray.push_back(p);
		}
		else if (SAME(token, "vt")){
			sscanf(next, "%f%f%f", &p.x, &p.y, &p.z);
		}
		else if (SAME(token, "f") || SAME(token, "fo")) {
			int		vtxCnt;
			char	vertexData[256];

			std::vector<int> pindices;

			for (vtxCnt = 0; vtxCnt < FACE_SIZE; vtxCnt++) {
				if (sscanf(next, "%s%n", vertexData, &width) != 1)
					break;
				next += width;

				pindices.push_back(atoi(vertexData) - 1);
			}

			polyarray.push_back(pindices);
		}

		strcpy(before_token, token);
	}

	fclose(objFile);

	return true;
}

#endif