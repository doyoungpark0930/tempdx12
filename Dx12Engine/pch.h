#pragma once

#include <windows.h>
#include <stdio.h>
#include <crtdbg.h>
#include <stdexcept>

#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_6.h> 
#include <d3dcompiler.h> // 셰이더 컴파일 시
#include <dxgidebug.h>
#include "d3dx12.h"

//직접 내 엔진 h에 포함
#include "SimpleMath.h"
#include "CommonStructs.h"

#define pi 3.141592

//메모리 누수 생겼을 때, 할당된 메모리의 출처(파일, 라인 번호)를 Output에 띄워주도록함
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif




