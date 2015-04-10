#include "obj_loader.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <cmath>

static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b);
static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token);
static inline unsigned int ParseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end);
static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end);
static inline std::vector<std::string> SplitString(const std::string &s, char delim);

OBJModel::OBJModel(const std::string& fileName)
{
	this->bezierMat[0][0] = -1.0f;
	this->bezierMat[0][1] = 3.0f;
	this->bezierMat[0][2] = -3.0f;
	this->bezierMat[0][3] = 1.0f;

	this->bezierMat[1][0] = 3.0f;
	this->bezierMat[1][1] = -6.0f;
	this->bezierMat[1][2] = 3.0f;
	this->bezierMat[1][3] = 0.0f;

	this->bezierMat[2][0] = -3.0f;
	this->bezierMat[2][1] = 3.0f;
	this->bezierMat[2][2] = 0.0f;
	this->bezierMat[2][3] = 0.0f;

	this->bezierMat[3][0] = 1.0f;
	this->bezierMat[3][1] = 0.0f;
	this->bezierMat[3][2] = 0.0f;
	this->bezierMat[3][3] = 0.0f;


	hasUVs = false;
	hasNormals = false;
	std::vector<glm::vec3> patchVectors;
	int patchCounter = 0;
    std::ifstream file;
    file.open(fileName.c_str());

    std::string line;
    if(file.is_open())
    {
        while(file.good())
        {
            getline(file, line);
        
            unsigned int lineLength = line.length();
            
            if(lineLength < 2)
                continue;
            
            const char* lineCStr = line.c_str();
            
            switch(lineCStr[0])
            {
                case 'v':
                    if(lineCStr[1] == 't')
                        this->uvs.push_back(ParseOBJVec2(line, lineCStr[2]));
                    else if(lineCStr[1] == 'n')
                        this->normals.push_back(ParseOBJVec3(line, lineCStr[2]));
                    else if(lineCStr[1] == ' ' || lineCStr[1] == '\t')
						this->vertices.push_back(ParseOBJVec3(line, lineCStr[1]));
                break;
                case 'f':
                    CreateOBJFace(line);
                break;
				case 'n':
					resolution = ParseOBJFloatValue(line, 2, 3);
                    //get resolution
                break;
				case 'b':
					patchCounter++;
					patchVectors.push_back(ParseOBJVec3(line, lineCStr[1]));

					if (patchCounter == 16)
					{
						AddNewGeometryMats(patchVectors);
						patchCounter = 0;
						patchVectors.clear();
					}
                    //add patch
                break;
                default: break;
            };
        }
    }
    else
    {
        std::cerr << "Unable to load mesh: " << fileName << std::endl;
    }

	if(!patches.empty())
	{
		for (int x = 0; x < patches.size(); x = x+3)
		{
			for(int i = 1; i < resolution; i++)
			{
				for(int j = 1; j < resolution; j++)
				{

					glm::vec3 vert_11;
					glm::vec3 vert_10;
					glm::vec3 vert_01;
					glm::vec3 vert_00;

					glm::vec3 face_1;
					glm::vec3 face_2;

					glm::vec4 U_1 = glm::vec4(pow((float)i/resolution, 3), pow((float)i/resolution, 2), (float)i/resolution, 1);
					glm::vec4 V_1 = glm::vec4(pow((float)j/resolution, 3), pow((float)j/resolution, 2), (float)j/resolution, 1);

					glm::vec4 U_0 = glm::vec4(pow(i-1/resolution, 3), pow(i-1/resolution, 2), i-1/resolution, 1);
					glm::vec4 V_0 = glm::vec4(pow(j-1/resolution, 3), pow(j-1/resolution, 2), j-1/resolution, 1);

					glm::vec4 Vecx_1 = bezierMat * patches[x] * bezierMat * V_1;
					glm::vec4 Vecy_1 = bezierMat * patches[x+1] * bezierMat * V_1;
					glm::vec4 Vecz_1 = bezierMat * patches[x+2] * bezierMat * V_1;

					glm::vec4 Vecx_0 = bezierMat * patches[x] * bezierMat * V_0;
					glm::vec4 Vecy_0 = bezierMat * patches[x+1] * bezierMat * V_0;
					glm::vec4 Vecz_0 = bezierMat * patches[x+2] * bezierMat * V_0;

					vert_11[0] = glm::dot(U_1, Vecx_1); // x
					vert_11[1] = glm::dot(U_1, Vecy_1); // y
					vert_11[2] = glm::dot(U_1, Vecz_1); // z

					vert_10[0] = glm::dot(U_1, Vecx_0); // x
					vert_10[1] = glm::dot(U_1, Vecy_0); // y
					vert_10[2] = glm::dot(U_1, Vecz_0); // z

					vert_01[0] = glm::dot(U_0, Vecx_1); // x
					vert_01[1] = glm::dot(U_0, Vecy_1); // y
					vert_01[2] = glm::dot(U_0, Vecz_1); // z

					vert_00[0] = glm::dot(U_0, Vecx_0); // x
					vert_00[1] = glm::dot(U_0, Vecy_0); // y
					vert_00[2] = glm::dot(U_0, Vecz_0); // z

				}
			}
		}
	}
}

void IndexedModel::CalcNormals()
{
    for(unsigned int i = 0; i < indices.size(); i += 3)
    {
        int i0 = indices[i];
        int i1 = indices[i + 1];
        int i2 = indices[i + 2];

        glm::vec3 v1 = positions[i1] - positions[i0];
        glm::vec3 v2 = positions[i2] - positions[i0];
        
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
            
        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }
    
    for(unsigned int i = 0; i < positions.size(); i++)
        normals[i] = glm::normalize(normals[i]);
}

IndexedModel OBJModel::ToIndexedModel()
{
    IndexedModel result;
    IndexedModel normalModel;
    
    unsigned int numIndices = OBJIndices.size();
    
    std::vector<OBJIndex*> indexLookup;
    
    for(unsigned int i = 0; i < numIndices; i++)
        indexLookup.push_back(&OBJIndices[i]);
    
    std::sort(indexLookup.begin(), indexLookup.end(), CompareOBJIndexPtr);
    
    std::map<OBJIndex, unsigned int> normalModelIndexMap;
    std::map<unsigned int, unsigned int> indexMap;
    
    for(unsigned int i = 0; i < numIndices; i++)
    {
        OBJIndex* currentIndex = &OBJIndices[i];
        
        glm::vec3 currentPosition = vertices[currentIndex->vertexIndex];
        glm::vec2 currentTexCoord;
        glm::vec3 currentNormal;
        
        if(hasUVs)
            currentTexCoord = uvs[currentIndex->uvIndex];
        else
            currentTexCoord = glm::vec2(0,0);
            
        if(hasNormals)
            currentNormal = normals[currentIndex->normalIndex];
        else
            currentNormal = glm::vec3(0,0,0);
        
        unsigned int normalModelIndex;
        unsigned int resultModelIndex;
        
        //Create model to properly generate normals on
        std::map<OBJIndex, unsigned int>::iterator it = normalModelIndexMap.find(*currentIndex);
        if(it == normalModelIndexMap.end())
        {
            normalModelIndex = normalModel.positions.size();
        
            normalModelIndexMap.insert(std::pair<OBJIndex, unsigned int>(*currentIndex, normalModelIndex));
            normalModel.positions.push_back(currentPosition);
            normalModel.texCoords.push_back(currentTexCoord);
            normalModel.normals.push_back(currentNormal);
        }
        else
            normalModelIndex = it->second;
        
        //Create model which properly separates texture coordinates
        unsigned int previousVertexLocation = FindLastVertexIndex(indexLookup, currentIndex, result);
        
        if(previousVertexLocation == (unsigned int)-1)
        {
            resultModelIndex = result.positions.size();
        
            result.positions.push_back(currentPosition);
            result.texCoords.push_back(currentTexCoord);
            result.normals.push_back(currentNormal);
        }
        else
            resultModelIndex = previousVertexLocation;
        
        normalModel.indices.push_back(normalModelIndex);
        result.indices.push_back(resultModelIndex);
        indexMap.insert(std::pair<unsigned int, unsigned int>(resultModelIndex, normalModelIndex));
    }
    
    if(!hasNormals)
    {
        normalModel.CalcNormals();
        
        for(unsigned int i = 0; i < result.positions.size(); i++)
            result.normals[i] = normalModel.normals[indexMap[i]];
    }
    
    return result;
};

unsigned int OBJModel::FindLastVertexIndex(const std::vector<OBJIndex*>& indexLookup, const OBJIndex* currentIndex, const IndexedModel& result)
{
    unsigned int start = 0;
    unsigned int end = indexLookup.size();
    unsigned int current = (end - start) / 2 + start;
    unsigned int previous = start;
    
    while(current != previous)
    {
        OBJIndex* testIndex = indexLookup[current];
        
        if(testIndex->vertexIndex == currentIndex->vertexIndex)
        {
            unsigned int countStart = current;
        
            for(unsigned int i = 0; i < current; i++)
            {
                OBJIndex* possibleIndex = indexLookup[current - i];
                
                if(possibleIndex == currentIndex)
                    continue;
                    
                if(possibleIndex->vertexIndex != currentIndex->vertexIndex)
                    break;
                    
                countStart--;
            }
            
            for(unsigned int i = countStart; i < indexLookup.size() - countStart; i++)
            {
                OBJIndex* possibleIndex = indexLookup[current + i];
                
                if(possibleIndex == currentIndex)
                    continue;
                    
                if(possibleIndex->vertexIndex != currentIndex->vertexIndex)
                    break;
                else if((!hasUVs || possibleIndex->uvIndex == currentIndex->uvIndex) 
                    && (!hasNormals || possibleIndex->normalIndex == currentIndex->normalIndex))
                {
                    glm::vec3 currentPosition = vertices[currentIndex->vertexIndex];
                    glm::vec2 currentTexCoord;
                    glm::vec3 currentNormal;
                    
                    if(hasUVs)
                        currentTexCoord = uvs[currentIndex->uvIndex];
                    else
                        currentTexCoord = glm::vec2(0,0);
                        
                    if(hasNormals)
                        currentNormal = normals[currentIndex->normalIndex];
                    else
                        currentNormal = glm::vec3(0,0,0);
                    
                    for(unsigned int j = 0; j < result.positions.size(); j++)
                    {
                        if(currentPosition == result.positions[j] 
                            && ((!hasUVs || currentTexCoord == result.texCoords[j])
                            && (!hasNormals || currentNormal == result.normals[j])))
                        {
                            return j;
                        }
                    }
                }
            }
        
            return -1;
        }
        else
        {
            if(testIndex->vertexIndex < currentIndex->vertexIndex)
                start = current;
            else
                end = current;
        }
    
        previous = current;
        current = (end - start) / 2 + start;
    }
    
    return -1;
}

void OBJModel::CreatePatch(const std::string& line)
{
    std::vector<std::string> tokens = SplitString(line, ' ');
	if((int)tokens.size() < 3)
	{
		tokens = SplitString(line, '\t');
	}


    this->OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));
    this->OBJIndices.push_back(ParseOBJIndex(tokens[2], &this->hasUVs, &this->hasNormals));
    this->OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));

    if((int)tokens.size() > 4)
    {
        this->OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));
        this->OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));
        this->OBJIndices.push_back(ParseOBJIndex(tokens[4], &this->hasUVs, &this->hasNormals));
    }
}

void OBJModel::CreateOBJFace(const std::string& line)
{
    std::vector<std::string> tokens = SplitString(line, ' ');
	if((int)tokens.size() < 3)
	{
		tokens = SplitString(line, '\t');
	}


    this->OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));
    this->OBJIndices.push_back(ParseOBJIndex(tokens[2], &this->hasUVs, &this->hasNormals));
    this->OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));

    if((int)tokens.size() > 4)
    {
        this->OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));
        this->OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));
        this->OBJIndices.push_back(ParseOBJIndex(tokens[4], &this->hasUVs, &this->hasNormals));
    }
}

OBJIndex OBJModel::ParseOBJIndex(const std::string& token, bool* hasUVs, bool* hasNormals)
{
    unsigned int tokenLength = token.length();
    const char* tokenString = token.c_str();
    
    unsigned int vertIndexStart = 0;
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    OBJIndex result;
    result.vertexIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    result.uvIndex = 0;
    result.normalIndex = 0;
    
    if(vertIndexEnd >= tokenLength)
        return result;
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    result.uvIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    *hasUVs = true;
    
    if(vertIndexEnd >= tokenLength)
        return result;
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    result.normalIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    *hasNormals = true;
    
    return result;
}

glm::vec3 OBJModel::ParseOBJVec3(const std::string& line, char delim) 
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();
    
    unsigned int vertIndexStart = 2;
    
    while(vertIndexStart < tokenLength)
    {
        if(tokenString[vertIndexStart] != delim && tokenString[vertIndexStart] != ' ' && tokenString[vertIndexStart] != '\t')
            break;
        vertIndexStart++;
    }
    
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, delim);
    
    float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, delim);
    
    float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, delim);
    
    float z = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    return glm::vec3(x,y,z);

    //glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()))
}

glm::vec2 OBJModel::ParseOBJVec2(const std::string& line, char delim)
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();
    
    unsigned int vertIndexStart = 3;
    
    while(vertIndexStart < tokenLength)
    {
        if(tokenString[vertIndexStart] != delim && tokenString[vertIndexStart] != ' ' && tokenString[vertIndexStart] != '\t')
            break;
        vertIndexStart++;
    }
    
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, delim);
    
    float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, delim);
    
    float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    return glm::vec2(x,y);
}

void OBJModel::AddNewGeometryMats(const std::vector<glm::vec3>& patchVectors)
{
	// Hard coded, but fuck it
	glm::mat4 Cx;
	glm::mat4 Cy;
	glm::mat4 Cz;

	for(int y=0; y < 4; y++)
	{
		for(int x=0; x < 4; x++)
		{
			Cx[y][x] = patchVectors[4 * y + x][0];
			Cy[y][x] = patchVectors[4 * y + x][1];
			Cz[y][x] = patchVectors[4 * y + x][2];
		}
	}

	patches.push_back(Cx);
	patches.push_back(Cy);
	patches.push_back(Cz);

}

static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b)
{
    return a->vertexIndex < b->vertexIndex;
}

static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token)
{
    unsigned int result = start;
    while(result < length)
    {
        result++;
        if(str[result] == token || str[result] == ' ' || str[result] == '\t')
            break;
    }
    
    return result;
}

static inline unsigned int ParseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end)
{
    return atoi(token.substr(start, end - start).c_str()) - 1;
}

static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end)
{
    return atof(token.substr(start, end - start).c_str());
}

static inline std::vector<std::string> SplitString(const std::string &s, char delim)
{
    std::vector<std::string> elems;
        
    const char* cstr = s.c_str();
    unsigned int strLength = s.length();
    unsigned int start = 0;
    unsigned int end = 0;
        
    while(end <= strLength)
    {
        while(end <= strLength)
        {
            if(cstr[end] == delim || cstr[end] == ' ' || cstr[end] == '\t')
                break;
            end++;
        }
            
        elems.push_back(s.substr(start, end - start));
        start = end + 1;
        end = start;
    }
        
    return elems;
}
