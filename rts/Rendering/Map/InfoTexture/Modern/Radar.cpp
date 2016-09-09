/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "Radar.h"
#include "Game/GlobalUnsynced.h"
#include "Map/Ground.h"
#include "Sim/Misc/LosHandler.h"


CRadarTexture::CRadarTexture()
: CPboInfoTexture("radar")
{
	texSize = losHandler->radar.size; //FIXME split radar & jammer
	texChannels = 2;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSpringTexStorage2D(GL_TEXTURE_2D, 1, GL_RG8, texSize.x, texSize.y);

	infoTexPBO.Bind();
	infoTexPBO.New(texSize.x * texSize.y * texChannels, GL_STREAM_DRAW);
	infoTexPBO.Unbind();
}


void CRadarTexture::Update()
{
	const unsigned short* myRadar       = &losHandler->radar.losMaps[gu->myAllyTeam].front();
	const unsigned short* myJammer      = &losHandler->jammer.losMaps[gu->myAllyTeam].front();

	infoTexPBO.Bind();
	auto infoTexMem = reinterpret_cast<unsigned char*>(infoTexPBO.MapBuffer());

	for (int y = 0; y < texSize.y; ++y) {
		for (int x = 0; x < texSize.x; ++x) {
			const int idx = (y * texSize.x) + x;

			const unsigned short* radarMap  = myRadar;
			const unsigned short* jammerMap = myJammer;

			infoTexMem[idx * 2 + 0] = ( radarMap[idx] != 0 || losHandler->globalLOS[gu->myAllyTeam]) ? 255 : 0;
			infoTexMem[idx * 2 + 1] = (jammerMap[idx] != 0                                 ) ? 255 : 0;
		}
	}

	infoTexPBO.UnmapBuffer();
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texSize.x, texSize.y, GL_RG, GL_UNSIGNED_BYTE, infoTexPBO.GetPtr());
	infoTexPBO.Invalidate();
	infoTexPBO.Unbind();
}
