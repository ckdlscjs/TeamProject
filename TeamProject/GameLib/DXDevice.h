#pragma once
#include "MyWindows.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class DXDevice
	: public MyWindows
{
public:
	virtual bool    Init();
	virtual bool    Frame();
	virtual bool    Render();
	virtual bool    Release();

	virtual HRESULT ResizeDevice(UINT width, UINT height);
	virtual HRESULT CreateDxResource();
	virtual HRESULT DeleteDxResource();

public:
	// 1)����̽� ����
	HRESULT CreateDevice();
	// 2) ���丮 ����
	HRESULT	CreateDXGIDevice();
	// 3) ����ü�� ����
	HRESULT CreateSwapChain();
	// 4) ����Ÿ�ٺ� ����
	HRESULT CreateRenderTargetView();
	// 5) ���� ���ٽ� �� ����
	HRESULT CreateDepthStencilView();
	// 6) ����Ʈ ����
	void CreateViewport();

	void ClearD3D11DeviceContext();

protected:
	ID3D11Device*				m_pd3dDevice = nullptr;
	ID3D11DeviceContext*		m_pImmediateContext = nullptr;
	IDXGIFactory*					m_pGIFactory = nullptr;


	IDXGISwapChain*				m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRTV = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
	D3D11_VIEWPORT			m_vp;
};

