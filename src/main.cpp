#include "libImpossibleAtlas.hpp"

#include <fstream>
#include <iostream>
#include <arpa/inet.h>
#include <filesystem>
#include <stdlib.h>
#include <math.h>

struct ImageFile
{
    int height;
    int width;
    std::string fileName;
};

struct atlasPos
{
    int x;
    int y;
    int h;
    int w;
}

//adapted from https://stackoverflow.com/questions/5354459/c-how-to-get-the-image-size-of-a-png-file-in-directory
ImageFile getImageDimensions(std::string file) { 
    std::ifstream in(file);
    unsigned int width, height;
    ImageFile result;

    in.seekg(16);
    in.read((char *)&width, 4);
    in.read((char *)&height, 4);

    result.width = ntohl(width);
    result.height = ntohl(height);
    result.fileName = file;

    return result;
}

int splitSheet(int numToProcess, char *toProcess[])
{
    for(int arg = 1; arg < numToProcess; arg++)
    {
        if(!std::filesystem::exists(toProcess[arg]))
        {
            std::cout << "File " << toProcess[arg] << " does not exist! Exiting..." << std::endl;
            return -1;
        }

        std::string firstArg(toProcess[arg]);
        ImageAtlas workingAtlas(firstArg, false);        

        Image* tempImage;
        Fragment tempFragment;
        ImageFile tempImageFile;
        int trueStartX, trueStartY, trueEndX, trueEndY, trueWidth, trueHeight;

        for(int i = 0; i < workingAtlas.getImagesCount(); i++)
        {
            tempImage = workingAtlas.getImageByIndex(0);
            if(tempImage != nullptr)
            {
                if(!std::filesystem::exists(tempImage->name_imageType_0))
                {
                    std::cout << "File " << tempImage->name_imageType_0 << " referenced by " << toProcess[arg] << " does not exist! Exiting..." << std::endl;
                    return -1;
                }
                ImageFile workingImage = getImageDimensions(tempImage->name_imageType_0);
                workingImage.fileName = tempImage->name_imageType_0;

                //adapted from https://stackoverflow.com/questions/10532384/how-to-remove-a-particular-substring-from-a-string
                std::string dirPath = tempImage->name_imageType_0;
                int start_position_to_erase = dirPath.find(".png");
                dirPath.erase(start_position_to_erase, 4);
                
                
                std::filesystem::create_directory(dirPath);

                for(int j = 0; j < tempImage->fragmentArrLen; j++)
                {
                    tempFragment = tempImage->FragmentArr[j];
                    trueStartX = std::round(workingImage.width * tempFragment.x_short_1);
                    trueStartY = std::round(workingImage.height * tempFragment.y_short_2);
                    trueEndX = std::round(workingImage.width * (tempFragment.x_short_1 + tempFragment.w_short_3));
                    trueEndY = std::round(workingImage.height * (tempFragment.y_short_2 + tempFragment.h_short_4));
                    trueWidth = trueEndX - trueStartX;
                    trueHeight = trueEndY - trueStartY;

                    std::string command = "magick -extract " + std::to_string(trueWidth) + "x" + std::to_string(trueHeight) + "+" + std::to_string(trueStartX) + "+" + std::to_string(trueStartY) + " " + tempImage->name_imageType_0 + " " + dirPath + "/" + tempFragment.name_utf_0 + ".png";

                    int success = system(command.data());
                }
            }

        }
    }

    return 1;
}

int mergeSheet(int numToProcess, char *toProcess[], int ssHeight, int ssWidth)
{
    //adapted from https://www.cppstories.com/2019/04/dir-iterate/
    ImageAtlas master(false);
    Image tempImageStruct;
    tempImageStruct.alpha = "CHANNEL";

    system("convert -size " + std::to_string(ssHeight) + "x" + std::to_string(ssWidth) + " xc:transparent png24:blank.png")

    for(int arg = 1; arg < numToProcess; arg++)
    {
        std::string path(toProcess[arg]); // Current directory
        tempImageStruct.name_utf_0 = path + ".png"
        if(!std::filesystem::exists(toProcess[arg]))
        {
            std::cout << "Directory " << toProcess[arg] << " does not exist! Exiting..." << std::endl;
            return -1;
        }

        //std::string firstArg(toProcess[arg]);
        //ImageAtlas workingAtlas(firstArg, false);  

        int tallestYPos = 0;
        int currentXPos = 0
        int currentYPos = 0;

        std::vector<std::string> filesInFolder;

        for(const auto&entry:std::filesystem::directory_iterator(path)) {
            if(entry.path().extension() == ".png")
            {
                filesInFolder.push_back(entry.path().filename());
            }
        }

        ImageFile tempImageFile;
        Fragment tempFragment;
        atlasPos trueDimensions;
        std::vector<atlasPos> fragmentLocations;

        for(int i = 0; i < filesInFolder.size(); i++)
        {
            tempImageFile = getImageDimensions(path + "/" + filesInFolder[i]);

            if(tempImageFile.height > ssHeight || tempImageFile.width > ssWidth)
            {
                std::cout << "ERROR: subimage " << tempImageFile.fileName << " is larger than the desired spritesheet size!";
                return -1;
            }

            //adapted from https://stackoverflow.com/questions/10532384/how-to-remove-a-particular-substring-from-a-string
            std::string fragmentName = tempImageFile.fileName;
            int start_position_to_erase = fragemntName.find(".png");
            fragmentName.erase(start_position_to_erase, 4);
            tempFragment.name_imageType_0 = fragmentName;

            if(tempImageFile.height > tallestYPos)
            {
                tallestYPos = tempImageFile.height;
            }

            //trueStartX = std::round(workingImage.width * tempFragment.x_short_1);

            if(currentXPos + tempImageFile.width > ssWidth)
            {
                currentXPos = 0;
                currentYPos = tallestYPos;
                tallestYPos = 0;
            }
            if(currentYPos + tempImageFile.height > ssHeight)
            {
                std::cout << "ERROR: reached end of spritesheet!";
                return -1;               
            }

            trueDimensions.x = currentXPos;
            trueDimensions.y = currentYPos;
            trueDimensions.w = tempImageFile.width;
            trueDimensions.h = tempImageFile.height;

            tempFragment.x_short_1 = currentXPos/ssWidth;
            tempFragment.y_short_2 = currentYPos/ssHeight;
            tempFragment.w_short_3 = (currentXPos + tempImageFile.width)/ssWidth;
            tempFragment.h_short_4 = (currentYPos + tempImageFile.height)/ssHeight;

            tempImageStruct.FragmentArr.push_back(tempFragment);
            fragmentLocations.push_back(trueDimensions);


            //actual spritesheet merging goes here
        }

        master.addImage(tempImageStruct);
        tempImageStruct.FragmentArr.clear();
        tempImageStruct.fragmentArrLen = 0;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cout << argv[0] << ": missing file operand" << std::endl;
        return -1;
    }
    else
    {
        return mergeSheet(argc, argv, 2048, 2048);
    }

}