#pragma once

#include "MiliToolBoxPrivatePCH.h"
#include "MiliToolVRUtil.h"


void MiliToolVRUtil::VRPython(const TArray<FString>& Args)
{
	hgLog("VRPython num args: %i",Args.Num());
	string pyCode = "";
	pyCode +="\r\nimport UEPyMili.Rendering.nuke360 as nuke360";
	pyCode +="\r\nnuke360.nuke360(img12=[";
	int count=0;
	for(auto& arg : Args) {
		pyCode += "\r\n       '" +util().ReplaceString(toString(arg),"*"," ")+"',";
		count ++;
		if(count>=12)
			break;
	}
	pyCode +="\r\n    ],";
	if(Args.Num()>12)  pyCode +="\r\n    outputPath='"+util().ReplaceString(toString(Args[12]), "*", " ") +"',";
	if(Args.Num()>13)  pyCode +="\r\n    outRes="+toString(Args[13]) +",";
	if(Args.Num()>14)  pyCode +="\r\n    nukePath=r'"+util().ReplaceString(toString(Args[14]),"*"," ") +"',";
	if(Args.Num()>19)  pyCode +="\r\n    motionBlend=["+toString(Args[15])+","+toString(Args[16])+","+toString(Args[17])+","+toString(Args[18])+","+toString(Args[19])+"],";
	pyCode +="\r\n)";
	hgLog("VRPython pyCode: \n%s", tchar(pyCode));
	util::Python(pyCode, 0);
    return;
}

void MiliToolVRUtil::MotionBlendPython(const TArray<FString>& Args)
{
	hgLog("VRPython num args: %i",Args.Num());
	string pyCode = "";
	pyCode +="\r\nimport UEPyMili.Rendering.nuke360 as nuke360";
	pyCode +="\r\nnuke360.motionBlendImages(imgPrefix='"+toString(Args[0])
							+"', imgSuffix='.png', numDigits=6, numDigits="+toString(Args[1])
							+", numImgs="+toString(Args[2])
							+", frameNum="+toString(Args[3])
							+", deleteSource="+toString(Args[4])
							+", flippingStereoOut="+toString(Args[5])
							+", nukePath=r'"+toString(Args[6])+"')"
		;
	hgLog("VRPython pyCode: \n%s",tchar(pyCode));
	util::Python(pyCode,1);
	return;
}