/*
    Copyright (c) 2013, NVIDIA CORPORATION. All rights reserved.
    Copyright (c) 2013, Tristan Lorach. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of NVIDIA CORPORATION nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
    OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Please direct any questions to tlorach@nvidia.com (Tristan Lorach)
*/
#ifdef WIN32
#include <windows.h> //for OutputDebugString
#endif
#include <ctype.h>
#include <fstream>
#include <assert.h>
#include "Fx.h"
#include "FxD3D.h"

#pragma warning(disable:4996)
using namespace nvFX;

/*****************************************************************************/
/* D3DShaderProgram D3DShaderProgram D3DShaderProgram D3DShaderProgram       */
/*****************************************************************************/

/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
D3DShaderProgram::~D3DShaderProgram()
{
    cleanup();

	D3DShaderProgram::ShaderData * allShaders[] = {
		&m_dataVS,
		&m_dataGS,
		&m_dataPS
	};
	ShaderType allShaderTypes[] = {
		FX_VTXPROG,
		FX_GEOMPROG,
		FX_FRAGPROG
	};

	for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
	{
		allShaders[scnt]->shaders.clear();
	}
}

/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
D3DShaderProgram::D3DShaderProgram(Container *pCont) : Program(pCont)
{
	m_linkNeeded = true;
    //m_usable = false;
    //m_bound = false;
    m_shaderFlags = 0;

	D3DShaderProgram::ShaderData * allShaders[] = {
		&m_dataVS,
		&m_dataGS,
		&m_dataPS
	};
	ShaderType allShaderTypes[] = {
		FX_VTXPROG,
		FX_GEOMPROG,
		FX_FRAGPROG
	};

	for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
	{
		allShaders[scnt]->compiledShader = NULL;
		allShaders[scnt]->reflector = NULL;
		allShaders[scnt]->shader = NULL;
	}
    // D3D is always separate
    pCont->separateShadersEnable(true);
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
IShader*    D3DShaderProgram::getShader(int n, ShaderType *t)
{
	// TODO avoroshilov: !!! add internal counter in the program, and correlate it with the shader, or something
	// TODO avoroshilov: currently, this implementation doesn't return shader by the number they were added

    ShaderMap::iterator it;

	D3DShaderProgram::ShaderData * allShaders[] = {
		&m_dataVS,
		&m_dataGS,
		&m_dataPS
	};
	ShaderType allShaderTypes[] = {
		FX_VTXPROG,
		FX_GEOMPROG,
		FX_FRAGPROG
	};


	for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
	{
		it = allShaders[scnt]->shaders.begin();
		while(it != allShaders[scnt]->shaders.end())
		{
			if(n == 0)
			{
				if(t){
#if 0
					switch(m_shaderFlags)
					{
					case FX_VERTEX_SHADER_BIT: *t = FX_VTXPROG; break;
					case FX_FRAGMENT_SHADER_BIT: *t = FX_VTXPROG; break;
					case FX_GEOMETRY_SHADER_BIT: *t = FX_GEOMPROG; break;
					case FX_TESS_CONTROL_SHADER_BIT: *t = FX_TCSPROG; break;
					case FX_TESS_EVALUATION_SHADER_BIT: *t = FX_TESPROG; break;
					default: *t = FX_UNKNOWNSHADER; break;
					}
#else
					*t = allShaderTypes[scnt];
#endif
				}
				return it->second;
			}
			--n;
			++it;
		}
	}
    return NULL;
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
IShader*    D3DShaderProgram::getShader(IShader *pShader, ShaderType *t)
{
    ShaderMap::iterator it;

	D3DShaderProgram::ShaderData * allShaders[] = {
		&m_dataVS,
		&m_dataGS,
		&m_dataPS
		};
	ShaderType allShaderTypes[] = {
		FX_VTXPROG,
		FX_GEOMPROG,
		FX_FRAGPROG
		};

	for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
	{
		it = allShaders[scnt]->shaders.begin();
		while(it != allShaders[scnt]->shaders.end())
		{
			if(it->second == pShader)
			{
				if(t){
#if 0
					switch(m_shaderFlags)
					{
					case FX_VERTEX_SHADER_BIT: *t = FX_VTXPROG; break;
					case FX_FRAGMENT_SHADER_BIT: *t = FX_VTXPROG; break;
					case FX_GEOMETRY_SHADER_BIT: *t = FX_GEOMPROG; break;
					case FX_TESS_CONTROL_SHADER_BIT: *t = FX_TCSPROG; break;
					case FX_TESS_EVALUATION_SHADER_BIT: *t = FX_TESPROG; break;
					case FX_COMPUTE_SHADER_BIT: *t = FX_COMPUTEPROG; break;
					default: *t = FX_UNKNOWNSHADER; break;
					}
#else
					*t = allShaderTypes[scnt];
#endif
				}
				return it->second;
			}
			++it;
		}
	}
    return NULL;
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
IShader*    D3DShaderProgram::getShader(const char *name, ShaderType *t)
{
    ShaderMap::iterator it;

	D3DShaderProgram::ShaderData * allShaders[] = {
		&m_dataVS,
		&m_dataGS,
		&m_dataPS
		};
	ShaderType allShaderTypes[] = {
		FX_VTXPROG,
		FX_GEOMPROG,
		FX_FRAGPROG
		};


	for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
	{
		it = allShaders[scnt]->shaders.find(name);
		if(it != allShaders[scnt]->shaders.end())
		{
			if(t){
#if 0
				switch(m_shaderFlags)
				{
				case FX_VERTEX_SHADER_BIT: *t = FX_VTXPROG; break;
				case FX_FRAGMENT_SHADER_BIT: *t = FX_FRAGPROG; break;
				case FX_GEOMETRY_SHADER_BIT: *t = FX_GEOMPROG; break;
				case FX_TESS_CONTROL_SHADER_BIT: *t = FX_TCSPROG; break;
				case FX_TESS_EVALUATION_SHADER_BIT: *t = FX_TESPROG; break;
				case FX_COMPUTE_SHADER_BIT: *t = FX_COMPUTEPROG; break;
				//case FX_PATH_CODE_BIT: *t = FX_PATHPROG; break;
				default: *t = FX_UNKNOWNSHADER; break;
				}
#else
				*t = allShaderTypes[scnt];
#endif
			}
			return it->second;
		}
	}
    return NULL;
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
int         D3DShaderProgram::getNumShaders()
{
    return
		m_dataVS.shaders.size() +
		m_dataGS.shaders.size() +
		m_dataPS.shaders.size();
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
bool D3DShaderProgram::addShader(ShaderType type, IShader* pShader, IContainer* pContainer)
{
    switch(type)
    {
    case FX_VTXPROG:
        //if(m_shaderFlags & (~FX_VERTEX_SHADER_BIT))
        //    return false;
        m_shaderFlags |= FX_VERTEX_SHADER_BIT;
        m_dataVS.shaders[std::string(pShader->getName())] = static_cast<D3DShader*>(pShader);
        break;
    case FX_FRAGPROG:
        //if(m_shaderFlags & (~FX_FRAGMENT_SHADER_BIT))
        //    return false;
        m_shaderFlags |= FX_FRAGMENT_SHADER_BIT;
        m_dataPS.shaders[std::string(pShader->getName())] = static_cast<D3DShader*>(pShader);
        break;
    case FX_GEOMPROG:
        //if(m_shaderFlags & (~FX_GEOMETRY_SHADER_BIT))
        //    return false;
        m_shaderFlags |= FX_GEOMETRY_SHADER_BIT;
        m_dataGS.shaders[std::string(pShader->getName())] = static_cast<D3DShader*>(pShader);
        break;
    case FX_TCSPROG:
        //if(m_shaderFlags & (~FX_TESS_CONTROL_SHADER_BIT))
        //    return false;
        m_shaderFlags |= FX_TESS_CONTROL_SHADER_BIT;
        //m_data.shaders[std::string(pShader->getName())] = static_cast<D3DShader*>(pShader);
		assert(0);
        break;
    case FX_TESPROG:
        //if(m_shaderFlags & (~FX_TESS_EVALUATION_SHADER_BIT))
        //    return false;
        m_shaderFlags |= FX_TESS_EVALUATION_SHADER_BIT;
        //m_data.shaders[std::string(pShader->getName())] = static_cast<D3DShader*>(pShader);
		assert(0);
		break;
    default:
assert(1);
        return false;
    }
    // add in the shader the target reference of this program
    //static_cast<Shader*>(pShader)->addTarget(this);
	static_cast<Shader*>(pShader)->addUser(this);
	return true;
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
bool D3DShaderProgram::addFragmentShader(IShader* pShader, IContainer* pContainer)
{
    return addShader(FX_FRAGPROG, pShader, pContainer);
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
bool D3DShaderProgram::addVertexShader(IShader* pShader, IContainer* pContainer)
{
    return addShader(FX_VTXPROG, pShader, pContainer);
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
//TODO: geom, tess...

void D3DShaderProgram::cleanup()
{
    /*ShaderVec::iterator icshd = m_commonShaders.begin();
    while(icshd != m_commonShaders.end())
    {
        (*icshd)->cleanupShader();
        ++icshd;
    }*/

#define CLEANUPSHD(shd)\
    if(shd.shader)         shd.shader->Release();\
    if(shd.compiledShader) shd.compiledShader->Release();\
    if(shd.reflector)      shd.reflector->Release();\
    shd.shader = NULL;\
    shd.compiledShader = NULL;\
    shd.code.clear();\
    shd.reflector = NULL;

	D3DShaderProgram::ShaderData * allShaders[] = {
		&m_dataVS,
		&m_dataGS,
		&m_dataPS
	};
	ShaderType allShaderTypes[] = {
		FX_VTXPROG,
		FX_GEOMPROG,
		FX_FRAGPROG
	};

	for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
	{
		ShaderMap::iterator iShd = allShaders[scnt]->shaders.begin();
		ShaderMap::iterator iEnd = allShaders[scnt]->shaders.end();
		for(;iShd != iEnd; iShd++)
		{
			//iShd.second->removeTarget(this);
			iShd->second->removeUser(this);
		}

		// TODO avoroshilov: see if more than one shader/reflector/etc should be released here
		CLEANUPSHD((*allShaders[scnt]));
	}

#undef CLEANUPSHD
}
/*************************************************************************/ /**
 ** 
 ** Linkage doesn't mean anything under D3D
 **
 **/ /*************************************************************************/ 
bool D3DShaderProgram::link(IContainer* pContainer)
{
    return true;
}
/*************************************************************************/ /**
 ** 
 ** translates line numbers to get proper # inside the effect files
 ** 
 **/ /*************************************************************************/ 
void D3DShaderProgram::replaceLineNumbers(char *strDest, int len, LPCSTR log, std::vector<D3DShader::codeInfo> &codeBlocks)
{
    strncpy(strDest, log, len);
#if 0
    //
    // Get Log even if things went well... for warnings or other messages
    //
    buf[0] = '\0';
    //glGetInfoLogARB(m_program, 1024, &len, buf);
    if(len)
    {
        char *p = strstr(buf, "0(");
        nvFX::printf("Log for D3DShaderProgram %d: %d %s...\n", m_program, len, buf);
        if(p==NULL)
            p = strstr(buf, "0:"); // iOS error display
        if(p==NULL)
            nvFX::printf("%s\n", buf);
        while(p)
        {
            *p = '\0';
            p+=2;
            char *pNum = p;
            while((*p != ')')&&(*p != ':'))
                p++;
            *p++ = '\0';
            int num = atoi(pNum);
#   ifdef ES2EMU
            num -= 139; // WHY ?!?!?!?!
#   endif
            num--;
            assert( num >= 0 );
            int offsetNum = num;
            std::vector<D3DShader::codeInfo>::iterator iCB;
            std::vector<D3DShader::codeInfo>::iterator iCBFound = codeBlocks.end();
            for(iCB = codeBlocks.begin(); iCB != codeBlocks.end(); iCB++)
                if(num >= iCB->lineinShader)
                {
                    if(num < (iCB->lineinShader + iCB->totalLinesInFX))
                    {
                        iCBFound = iCB;
                        offsetNum = num - iCB->lineinShader;
                        break;
                    }
                }
            if/*assert*/( iCBFound != codeBlocks.end() )
                nvFX::printf("%s(%d)", iCBFound->fname.c_str(), iCBFound->lineInFX + offsetNum);
            else
                nvFX::printf("()");
            pNum = strstr(p, "0(");
            if(pNum==NULL)
                pNum = strstr(p, "0:");
            if(pNum && (!isalnum(pNum[2])))
                pNum = NULL;
            if(pNum) *pNum='\0';
            nvFX::printf("%s", p);
            p = pNum;
        }
        //else nvFX::printf("Log for %d:\n%s\n", shaderObj, buf);
    }
#endif
}
/*************************************************************************/ /**
 ** 
 ** TODO: in SHADERCONCAT_USE, add error log with proper line numbers to nvFX files
 ** 
 **/ /*************************************************************************/ 
bool D3DShaderProgram::bind(IContainer* pContainer)
{
    char buf[1024];
    bool err = false;
    HRESULT hr;
    ID3D1XBlob *shaderErrors;

    //if(m_bound) To remove... can't do this
    //    return true;
    //m_bound = true;

    ID3D1XDevice *pd3d1X = (ID3D1XDevice *)nvFX::getDevice();
#ifdef USE_D3D11
    ID3D11DeviceContext *pd3dDC;
    pd3d1X->GetImmediateContext(&pd3dDC);
#endif

    if(m_linkNeeded)
    {
		D3DShaderProgram::ShaderData * allShaders[] = {
			&m_dataVS,
			&m_dataGS,
			&m_dataPS
		};
		ShaderType allShaderTypes[] = {
			FX_VTXPROG,
			FX_GEOMPROG,
			FX_FRAGPROG
		};
		int allShaderFlags[] = {
			FX_VERTEX_SHADER_BIT,
			FX_GEOMETRY_SHADER_BIT,
			FX_FRAGMENT_SHADER_BIT
		};

		for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
		{
			if ((m_shaderFlags & allShaderFlags[scnt]) == 0)
				continue;

			D3DShaderProgram::ShaderData & curShaderData = *allShaders[scnt];

			//
			// add common parts for line-number conversion :-(
			//
			std::vector<D3DShader::codeInfo> codeBlocks;
			int lineOffset = 0;
			ShaderMap::iterator iShd, iEnd;
			iShd = curShaderData.shaders.begin();
			iEnd = curShaderData.shaders.end();
			for(;iShd != iEnd; iShd++)
			{
				for(int n=0; n<(int)iShd->second->m_startLineNumbers.size(); n++)
					codeBlocks.push_back(iShd->second->m_startLineNumbers[n]);
			}
			curShaderData.code.clear();
			// Add common header code
			D3DShader *pShd;
			for(int i=0; pShd = static_cast<D3DShader*>(pContainer->findShader(i)); i++)
				if(*(pShd->getName()) == '\0')
				{
					if(curShaderData.shaders.size()>0)
						curShaderData.code += pShd->getShaderCode();
				}
			if(curShaderData.shaders.size()>0)
			{
				iShd = curShaderData.shaders.begin();
				iEnd = curShaderData.shaders.end();
				for(;iShd != iEnd; iShd++)
					curShaderData.code += iShd->second->getShaderCode();
			}
			//
			// Compile the vertex shader
			//
			const char *profile;
#pragma MESSAGE(__FILE__"(298) : TODO TODO TODO TODO TODO : allow to chose the shader models!")
#if 0
			switch(m_shaderFlags)
			{
			case FX_VERTEX_SHADER_BIT: profile = "vs_4_0"; break;
			case FX_FRAGMENT_SHADER_BIT: profile = "ps_4_0"; break;
			case FX_GEOMETRY_SHADER_BIT: profile = "gs_4_0"; break;
			case FX_TESS_CONTROL_SHADER_BIT: profile = "hs_5_0"; break;
			case FX_TESS_EVALUATION_SHADER_BIT: profile = "ds_5_0"; break;
			default: profile = NULL; assert(1); break;
			}
#else
			switch(allShaderTypes[scnt])
			{
			case FX_VTXPROG: profile = "vs_4_0"; break;
			case FX_FRAGPROG: profile = "ps_4_0"; break;
			case FX_GEOMPROG: profile = "gs_4_0"; break;
			case FX_TCSPROG: profile = "hs_5_0"; break;
			case FX_TESPROG: profile = "ds_5_0"; break;
			default: profile = NULL; assert(1); break;
			}
#endif
			/*if*/assert(curShaderData.shaders.size()>0);
			{
				hr = D3DX1XCompileFromMemory(
				  curShaderData.code.c_str(), curShaderData.code.size(), "Shader",
				  NULL,//CONST D3D1X_SHADER_MACRO*
				  NULL, //LPD3D1XINCLUDE pInclude
				  "main",	// TODO avoroshilov: allow selecting different entry points
				  profile,
				  0,//Flags1
				  0,//Flags2
				  NULL,//ID3DX10ThreadPump *pPump
				  &curShaderData.compiledShader,
				  &shaderErrors, NULL);
		//OutputDebugStringA(m_data.code.c_str());
				if(FAILED(hr))
				{
					err = true;
					LPCSTR p = (LPCSTR)shaderErrors->GetBufferPointer();
					replaceLineNumbers(buf, 1024, p, codeBlocks);
					nvFX::printf("Log for Vtx Shader: \n%s\n", buf);
				}
				else
				{
					switch(m_shaderFlags)
					{
					case FX_VERTEX_SHADER_BIT:
						hr = pd3d1X->CreateVertexShader(curShaderData.compiledShader->GetBufferPointer(), curShaderData.compiledShader->GetBufferSize(), 
						#ifdef USE_D3D11
						NULL, // This is ID3D11ClassLinkage pointer. TODO: see how to use it later.
						#endif
						&curShaderData.vtxShader);
						break;
					case FX_FRAGMENT_SHADER_BIT:
						hr = pd3d1X->CreatePixelShader(curShaderData.compiledShader->GetBufferPointer(), curShaderData.compiledShader->GetBufferSize(), 
						#ifdef USE_D3D11
						NULL, // This is ID3D11ClassLinkage pointer. TODO: see how to use it later.
						#endif
						&curShaderData.pixShader);
						break;
					case FX_GEOMETRY_SHADER_BIT:
						hr = pd3d1X->CreateGeometryShader(curShaderData.compiledShader->GetBufferPointer(), curShaderData.compiledShader->GetBufferSize(), 
						#ifdef USE_D3D11
						NULL, // This is ID3D11ClassLinkage pointer. TODO: see how to use it later.
						#endif
						&curShaderData.gsShader);
						break;
					case FX_TESS_CONTROL_SHADER_BIT:
						//#ifdef USE_D3D11
						//hr = pd3d1X->CreateHullShader(curShaderData.compiledShader->GetBufferPointer(), curShaderData.compiledShader->GetBufferSize(), 
						//NULL, // This is ID3D11ClassLinkage pointer. TODO: see how to use it later.
						//&curShaderData.hullShader);
						//#else
						assert(1);
						//#endif
					case FX_TESS_EVALUATION_SHADER_BIT:
						//#ifdef USE_D3D11
						//hr = pd3d1X->CreateDomainShader(curShaderData.compiledShader->GetBufferPointer(), curShaderData.compiledShader->GetBufferSize(), 
						//NULL, // This is ID3D11ClassLinkage pointer. TODO: see how to use it later.
						//&curShaderData.evalShader);
						//#else
						assert(1);
						//#endif
					}
					if(FAILED(hr))
					{
						err = true;
						LPCSTR p = (LPCSTR)shaderErrors->GetBufferPointer();
						replaceLineNumbers(buf, 1024, p, codeBlocks);
						nvFX::printf("Shader creation error : \n%s\n", p);
					}
					// IID_ID3D1XShaderReflection doesn't work... WTF ?!?
					hr = D3DReflect( curShaderData.compiledShader->GetBufferPointer(), curShaderData.compiledShader->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&curShaderData.reflector);
					if(FAILED(hr))
					{
						err = true;
						nvFX::printf("Shader reflector failed\n");
					}
					//
					// Let's check if the shader forgot some parameters in $Globals cbuffer
					//
					D3D11_SHADER_DESC sd;
					ID3D1XShaderReflectionConstantBuffer *pCst = NULL;
					curShaderData.reflector->GetDesc((D3D1X_SHADER_DESC*)&sd);
					D3D1X_SHADER_BUFFER_DESC bufDesc;
					bufDesc.Name = NULL;
					pCst = curShaderData.reflector->GetConstantBufferByName("$Globals");
					pCst->GetDesc(&bufDesc);
					if(bufDesc.Name && (!strcmp(bufDesc.Name, "$Globals")))
					{
						nvFX::printf("WARNING: some uniform parameters are outside of any cbuffer and won't be initialized properly : \n");
						for(int i=0; i<(int)bufDesc.Variables; i++)
						{
							ID3D1XShaderReflectionVariable* pVar = pCst->GetVariableByIndex(i);
							D3D11_SHADER_VARIABLE_DESC vd; // because of a bug...?!?
							pVar->GetDesc((D3D1X_SHADER_VARIABLE_DESC*)&vd);
							nvFX::printf("%s\n", vd.Name);
						}
					}

					if(shaderErrors)
						shaderErrors->Release();
				}
			} //if(m_data.shaders.size()>0)
			if(err)
				return false;
			else
				m_linkNeeded = false;
		}
    } //if(m_linkNeeded)



    //
    // Activate the shaders
    //
#ifdef USE_D3D11
    // TODO: see how to take advantage of ID3D11ClassInstance * argument !
#if 0
    switch(m_shaderFlags)
    {
    case FX_VERTEX_SHADER_BIT:
        pd3dDC->VSSetShader(m_dataVS.vtxShader, NULL, 0);
        break;
    case FX_FRAGMENT_SHADER_BIT:
        pd3dDC->PSSetShader(m_dataPS.pixShader, NULL, 0);
        break;
    case FX_GEOMETRY_SHADER_BIT:
        pd3dDC->GSSetShader(m_dataGS.gsShader, NULL, 0);
        break;
    case FX_TESS_CONTROL_SHADER_BIT:
        //pd3dDC->...SetShader(m_data.);
        //break;
    case FX_TESS_EVALUATION_SHADER_BIT:
        //pd3dDC->...SetShader(m_data.);
        //break;
    default:
        assert(1);
    }
#else
	// We need to un-set shaders probably to not pollute state, but maybe it is not needed because 'unbind'
	bool isVS = (m_shaderFlags & FX_VERTEX_SHADER_BIT) != 0;
	bool isHS = (m_shaderFlags & FX_TESS_CONTROL_SHADER_BIT) != 0;
	bool isDS = (m_shaderFlags & FX_TESS_EVALUATION_SHADER_BIT) != 0;
	bool isGS = (m_shaderFlags & FX_GEOMETRY_SHADER_BIT) != 0;
	bool isPS = (m_shaderFlags & FX_FRAGMENT_SHADER_BIT) != 0;

	pd3dDC->VSSetShader(isVS ? m_dataVS.vtxShader : NULL, NULL, 0);
	pd3dDC->GSSetShader(isGS ? m_dataGS.gsShader : NULL, NULL, 0);
	pd3dDC->PSSetShader(isPS ? m_dataPS.pixShader : NULL, NULL, 0);
#endif
#else
    switch(m_shaderFlags)
    {
    case FX_VERTEX_SHADER_BIT:
        pd3d1X->VSSetShader(m_data.vtxShader);
        break;
    case FX_FRAGMENT_SHADER_BIT:
        pd3d1X->PSSetShader(m_data.pixShader);
        break;
    case FX_GEOMETRY_SHADER_BIT:
        pd3d1X->GSSetShader(m_data.gsShader);
        break;
    default:
        assert(1);
    }
#endif
    return true;
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
void D3DShaderProgram::unbind(IContainer* pContainer)
{
    //m_bound = false;
    ID3D1XDevice *pd3d1X = (ID3D1XDevice *)nvFX::getDevice();
#ifdef USE_D3D11
    ID3D11DeviceContext *pd3dDC;
    pd3d1X->GetImmediateContext(&pd3dDC);
    pd3dDC->VSSetShader(NULL, NULL, 0);
    pd3dDC->PSSetShader(NULL, NULL, 0);
    pd3dDC->GSSetShader(NULL, NULL, 0);
    // TODO: Other shaders...
#else
    pd3d1X->VSSetShader(NULL);
    pd3d1X->PSSetShader(NULL);
    pd3d1X->GSSetShader(NULL);
#endif
}

/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
int D3DShaderProgram::getUniformLocation(const char* name)
{
    return 0;//glGetUniformLocation(m_program, name);
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 

void D3DShaderProgram::setUniform(const char* name, float val)
{
	//glUniform1f(glGetUniformLocation(m_program, name), val);
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
void D3DShaderProgram::setUniform(const char* name, int val)
{
    //glUniform1i(glGetUniformLocation(m_program, name), val);
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
void D3DShaderProgram::setUniformVector(const char * name, const float* val, int count, int numUniforms)
{
    GLint location = 0;//glGetUniformLocation(m_program, name);
    printf("location for %s = %d\n", name, location);
    if (location >= 0) 
    {
        switch (count) {
            case 1:
                //glUniform1fv(location, numUniforms, val);
                break;
            case 2:
                //glUniform2fv(location, numUniforms, val);
                break;
            case 3:
                //glUniform3fv(location, numUniforms, val);
                break;
            case 4:
                //glUniform4fv(location, numUniforms, val);
                break;
        }
    } else {
        printf("BAD uniform offset for %s!\n", name);
    }
    //Err check...
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
void D3DShaderProgram::setUniformVector(const char * name, const int* val, int count, int numUniforms)
{
    GLint location = 0;//glGetUniformLocation(m_program, name);
    if (location == -1) {
        return;
    }
    switch (count) {
        case 1:
            //glUniform1iv(location, numUniforms, val);
            break;
        case 2:
            //glUniform2iv(location, numUniforms, val);
            break;
        case 3:
            //glUniform3iv(location, numUniforms, val);
            break;
        case 4:
            //glUniform4iv(location, numUniforms, val);
            break;
    }
}
/*************************************************************************/ /**
 ** 
 ** 
 **/ /*************************************************************************/ 
void D3DShaderProgram::setSampler(const char * texname, int i)
{
    GLint linked = 1;
    //glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (linked != 1) {
        return;
    }
    GLint location = 0;//glGetUniformLocation(m_program, texname);
    if (location == -1) {
        return;
    }
    //glUniform1i(location, i);
}
/*************************************************************************/ /**
 ** TODO: remove this method...
 ** we can't assign separately sub-routines... we must update all at the same time
 **/ /*************************************************************************/ 
void D3DShaderProgram::bindTexture(ResourceType target, const char * texname, GLuint texid, int i)
{
    //glActiveTexture(GL_TEXTURE0 + i);
    //glBindTexture(target, texid);
    setSampler(texname, i);
    //glActiveTexture(GL_TEXTURE0);
}

/*************************************************************************/ /**
 ** \brief associate to a sub-routine uniform ID some sub-routine implementations
 **
 ** this method is not supposed to be called every frame. It should be called
 ** during init time. tables (m_srIndicesVS etc.) will contain the proper relations
 ** that will be invoked at bind() time (via glUniformSubroutinesuiv)
 ** \todo This function could double check possible errors and return details
 ** for now, if wrong binding, bind() will generate an error (via glUniformSubroutinesuiv)
 **/ /*************************************************************************/ 
bool D3DShaderProgram::bindSubRoutineToUniform(int uniformID, GLenum shadertype, char **subroutineNames, int numNames)
{
    return false;
}
int D3DShaderProgram::getSubRoutineID(const char *name, GLenum shadertype)
{
    return -1;
}

int  D3DShaderProgram::getAttribLocation(const char* attrName)
{
DebugBreak(); // TO TEST

	D3DShaderProgram::ShaderData * allShaders[] = {
		&m_dataVS,
		&m_dataGS,
		&m_dataPS
	};
	ShaderType allShaderTypes[] = {
		FX_VTXPROG,
		FX_GEOMPROG,
		FX_FRAGPROG
	};

	for (int scnt = 0, scntend = sizeof(allShaders) / sizeof(void *); scnt < scntend; ++scnt)
	{
		D3DShaderProgram::ShaderData & curShaderData = *allShaders[scnt];

		if(!curShaderData.reflector)
			return -1;
		D3D1X_SHADER_DESC sd;
		curShaderData.reflector->GetDesc(&sd);
		for(int i=0; i<(int)sd.InputParameters; i++)
		{
			D3D1X_SIGNATURE_PARAMETER_DESC ipd;
			curShaderData.reflector->GetInputParameterDesc(i, &ipd);
			if(!strcmp(ipd.SemanticName, attrName))
				return i;
			char s[50];
			sprintf_s(s, 49, "%s%d", ipd.SemanticName, ipd.SemanticIndex);
			if(!strcmp(s, attrName))
				return i;
		}
	}
    return -1;
}
void D3DShaderProgram::bindAttribLocation(int i, const char* attrName)
{
}

void* D3DShaderProgram::getD3DIASignature()
{
    if(m_dataVS.compiledShader && (m_shaderFlags & FX_VERTEX_SHADER_BIT))
        return m_dataVS.compiledShader->GetBufferPointer();
    return NULL;
}
int D3DShaderProgram::getD3DIASignatureSize()
{
    if(m_dataVS.compiledShader && (m_shaderFlags & FX_VERTEX_SHADER_BIT))
        return m_dataVS.compiledShader->GetBufferSize();
    return 0;
}

