#ifndef CloudType_IO
#define CloudType_IO

#include <pcl/io/pcd_io.h>
#include <pcl/io/obj_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/TextureMesh.h>

#include <iostream>
#include <fstream>
using namespace std;

typedef pcl::PointXYZRGBA PointType;
typedef pcl::PointCloud<PointType> CloudType;
typedef pcl::Vertices Face;

class CloudIO
{
public:

	/* IO PCD */

	static CloudType::Ptr loadCloudFromPCD(string PCDFileName) {
		CloudType::Ptr cloudTarget(new CloudType);

		if (PCDFileName.size()) {
			pcl::io::loadPCDFile (PCDFileName, *cloudTarget);
			if (cloudTarget->points.size()) {
				std::cout << "Loaded " << cloudTarget->size() << " data points from " <<  PCDFileName << std::endl;
			} else {
				std::cerr << "-> ERROR: Couldn't load the point cloud!" << std::endl;
			}
		} else {
			std::cerr << "-> ERROR: Please put a file name with at least 1 (one) character!" << std::endl;
		}

		return cloudTarget;
	}

	static void saveCloudToPCD(CloudType::Ptr cloudSource, string PCDFileName = "point_cloud.pcd") {
		if (PCDFileName.size()) {
			pcl::io::savePCDFileASCII (PCDFileName, *cloudSource);
			std::cout << "Saved " << cloudSource->size() << " data points to " <<  PCDFileName << std::endl;
		} else {
			std::cerr << "-> ERROR: Please put a file name with at least 1 (one) character!" << std::endl;
		}
	}

	/* IO OFF */

	static CloudType::Ptr loadCloudFromOFF(string OFFFileName) {
		CloudType::Ptr cloud(new CloudType);
		ifstream file(OFFFileName.c_str());
		string offType;

		int nPoints = 0, nFaces = 0, nEdges = 0;

		// read number of points, faces and edges of the cloud
		getline(file, offType);
		file >> nPoints >> nFaces >> nEdges;

		// read points data
		for (int i = 0; i < nPoints; ++i)
		{
			PointType p;
			int temp_r = 255, temp_g = 255, temp_b = 255, temp_a = 0;

			file >> p.x >> p.y >> p.z;

			if (offType == "COFF")
			{
				file >> temp_r >> temp_g >> temp_b >> temp_a;
			}

			p.r = temp_r; p.g = temp_g; p.b = temp_b; p.a = temp_a;

			cloud->push_back(p);
		}

		file.close();
		return cloud;
	}

	static void saveCloudToOFF(CloudType::Ptr cloudSource, vector<Face> faces, string fileName = "point_cloud.off") {
		if (fileName.size())
		{
			ofstream file(fileName.c_str());
			int numValidPoints = 0;

			// write header
			file << "COFF" << endl;

			// number of not NaN points
			for (int i = 0; i < cloudSource->size(); ++i)
			{
				PointType p = cloudSource->at(i);
				if (not isnan(p.x) and not isnan(p.y) and not isnan(p.z))
				{
					numValidPoints++;
				}
			}

			file << numValidPoints << " " << faces.size() << " " << 0 << endl;

			// write points
			for (int i = 0; i < cloudSource->size(); ++i)
			{
				PointType p = cloudSource->at(i);
				if (not isnan(p.x) and not isnan(p.y) and not isnan(p.z))
				{
					file << float(p.x) << " " << float(p.y) << " " << float(p.z) << " " << int(p.r) << " " << int(p.g) << " " << int(p.b) << " " << int(p.a) << endl;
				}
			}

			// write faces
			for (int i = 0; i < faces.size(); ++i)
			{
				std::vector<unsigned int> f = faces[i].vertices;
				file << f.size() << " ";
				for (int j = 0; j < f.size(); ++j)
				{
					file << f[j] << " ";
				}
				file << endl;
			}

			file.close();

			cout << "Saved " << numValidPoints << " data points to " <<  fileName << endl;
		} else {
			cerr << "-> ERROR: Please put a file name with at least 1 (one) character!" << endl;
		}
	}

	/* IO OBJ */

	static pcl::TextureMesh loadCloudFromOBJ(string fileName) {
		pcl::TextureMesh texMesh;
		pcl::io::loadOBJFile(fileName, texMesh);
		return texMesh;
	}

	static void saveCloudToOBJ(pcl::TextureMesh texMesh, string fileName = "point_cloud.obj") {
		pcl::io::saveOBJFile(fileName, texMesh, 6);
	}

	static void fixFaces(string fileName) {
		string line;
		ifstream file(fileName.c_str());
		ofstream temp("temp.txt");
		int i = 0;

		if (file.is_open())
		{
			while ( getline (file,line) )
			{
				Tokenizer tokenizer(line, ' ');
				std::vector<string> lineSplited = tokenizer.remaining();
				std::vector<int> lineIndices;

				if (lineSplited[0] == "f")
				{
					Tokenizer subTokenizer1(lineSplited[1], '/');
					string indexStr = subTokenizer1.remaining()[0];
					int index = atoi(indexStr.c_str());
					lineIndices.push_back(index);

					Tokenizer subTokenizer2(lineSplited[2], '/');
					indexStr = subTokenizer2.remaining()[0];
					index = atoi(indexStr.c_str());
					lineIndices.push_back(index);

					Tokenizer subTokenizer3(lineSplited[3], '/');
					indexStr = subTokenizer3.remaining()[0];
					index = atoi(indexStr.c_str());
					lineIndices.push_back(index);

					temp << "f";
					for (int i = 0; i < lineIndices.size(); ++i)
					{
						int index = lineIndices[i];
						temp << " " << index << "/" << index << "/" << index;
					}
				} else {
					temp << line;
				}

				temp << endl;
			}
			file.close();
			temp.close();

			std::remove(fileName.c_str());
			std::rename("temp.txt", fileName.c_str());
		}
		else cout << "Unable to open file";
	}

	/* IO PLY */

	static CloudType::Ptr loadCloudFromPLY(string fileName) {
		CloudType::Ptr result(new CloudType);
		pcl::io::loadPLYFile (fileName, *result);
		return result;
	}

	static void saveCloudToPLY(CloudType::Ptr cloudSource, string fileName = "point_cloud.ply") {
		std::vector<int> v;
		CloudType::Ptr cloudSource_copy(new CloudType);
		pcl::removeNaNFromPointCloud(*cloudSource, *cloudSource_copy, v);

		pcl::io::savePLYFile (fileName, *cloudSource_copy);
	}

	/* converters */

	static void convertPCDtoOFF(string PCDFileName) {
		CloudType::Ptr cloud = loadCloudFromPCD(PCDFileName);
		saveCloudToOFF(cloud, std::vector<Face>(), PCDFileName + ".off");
	}

	static void convertOFFtoPCD(string OFFFileName) {
		CloudType::Ptr cloud = loadCloudFromOFF(OFFFileName);
		saveCloudToPCD(cloud, OFFFileName + ".pcd");
	}

	/* other */

	static vector<Face> loadFacesFromOFF(string OFFFileName) {
		ifstream file(OFFFileName.c_str());
		string line;
		int nPoints = 0, nFaces = 0, nEdges = 0;

		getline(file, line);

		file >> nPoints >> nFaces >> nEdges; getline(file, line);

		for (int i = 0; i < nPoints; ++i)
		{
			getline(file, line);
		}

		vector<Face> faces;

		for (int i = 0; i < nFaces; ++i)
		{
			int nVertices;
			file >> nVertices;

			Face face;
			
			for (int j = 0; j < nVertices; ++j)
			{
				int verticeIndex;
				file >> verticeIndex;
				face.vertices.push_back(verticeIndex);
			}

			faces.push_back(face);
		}

		file.close();
		return faces;
	}
};

#endif