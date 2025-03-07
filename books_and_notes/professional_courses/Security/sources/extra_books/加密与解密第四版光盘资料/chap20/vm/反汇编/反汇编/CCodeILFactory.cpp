/*-----------------------------------------------------------------------
第20章  虚拟机的设计
《加密与解密（第四版）》
(c)  看雪学院 www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

// VMPackLib

#include "stdafx.h"
#include "CCodeILFactory.h"

CCodeILFactory::CCodeILFactory()
{
}


void CCodeILFactory::Init(DWORD VirtualAddress)
{
	int nJumpTableAddr	= VirtualAddress;
	int nCodeEngineAddr = nJumpTableAddr  + JumpTableLen;
	int nEnterStubAddr	= nCodeEngineAddr + CodeEngineLen;
	int mVMEnterStubCodeAddr = nEnterStubAddr  + EnterStubAddrLen;
	int nVMCodeAddr		= mVMEnterStubCodeAddr  + VMEnterCodeLen;

	m_JumpTable.CreateVirtualMemory(nJumpTableAddr,JumpTableLen);
	m_CodeEngine.CreateVirtualMemory(nCodeEngineAddr,CodeEngineLen);
	m_EnterStub.CreateVirtualMemory(nEnterStubAddr,EnterStubAddrLen);
	m_VMEnterStubCode.CreateVirtualMemory(mVMEnterStubCodeAddr,VMEnterCodeLen);
	m_VMCode.CreateVirtualMemory(nVMCodeAddr,VMCodeLen);

	VMFactory.SetupVirtualMemory(&m_JumpTable,&m_CodeEngine,&m_EnterStub,&m_VMEnterStubCode,&m_VMCode);

	VMFactory.InitVM();
}
// 反汇编代码,识别出函数结束的地方.添加到提供的链表中
// CodeList : 输出汇编链表
// base_addr: 2进制代码
// VirtualAddress : 虚拟地址
void CCodeILFactory::DisasmFunction(list<CodeNode*> *CodeList,char* base_addr,DWORD VirtualAddress)
{
	DWORD Start_VirtualAddress = VirtualAddress;//第一个地址
	DWORD End_VirtualAddress = 0;//最后一句的地址

	t_disasm da;
	CodeNode* code = NULL;
	int len =0;
	int KeepNum = 0;
	//计算出函数的结尾
	while( TRUE )
	{
		code = new CodeNode;
		len = Disasm(base_addr,MAXCMDSIZE,VirtualAddress,&da,DISASM_CODE);

		memcpy(&code->disasm,&da,sizeof(t_disasm));
		//CodeList->AddTail(code);
		CodeList->push_back(code);

		if( KeepNum >= len && KeepNum != 0 )
			KeepNum -= len;

		if( IsJumpCmd(&da) )//是跳转指令
		{
			if( da.jmpconst && da.jmpconst > VirtualAddress )
			{
				KeepNum = da.jmpconst - VirtualAddress;
			}
		}
		if( _stricmp(da.result,"int3") == 0 )//如果找到int3
		{
			break;
		}
		if( _stricmp(da.result,"retn") == 0 )//如果找到返回指令
		{
			if( KeepNum - len <= 0 )//如果没有字节了.则返回
				break;
		}
		base_addr+=len;
		VirtualAddress+=len;
	}
	End_VirtualAddress = VirtualAddress;

	//计算出各语句的用处
	BOOL NextJMCALL = FALSE;
	//T_Link* pos = CodeList->GetLink();
	list<CodeNode*>::iterator itr;
	for( itr = CodeList->begin(); itr != CodeList->end(); itr++ )
	{
		CodeNode* code = *itr;
		t_disasm t_da ;
		memcpy(&t_da,&code->disasm,sizeof(t_disasm));
		if(code)
		{
			if( NextJMCALL )
			{
				NextJMCALL = FALSE;
				code->IsJmcNextType = TRUE;
			}
			if( IsJumpCmd(&t_da) )//是跳转指令
			{
				NextJMCALL = TRUE;
				if( t_da.jmpconst )
				{
					if( t_da.jmpconst > Start_VirtualAddress && t_da.jmpconst < End_VirtualAddress )//在LIST里
					{
						CodeNode* tmpNode = SearchAddrAsList(CodeList,code);//搜索链表
						if( tmpNode )
						{
							tmpNode->IsJmcFromType = TRUE;//标明这一句为从其他地方跳转过来的代码
						}
					}
					else
					{
						code->IsJmcBeSideType = TRUE;//为跳到外部的指令
					}
				}
				else//没有跳转地址,就是动态跳转
				{
					code->IsJmcDynamicType = TRUE;
				}
			}
			else if( IsCallCmd(&t_da) )//是函数调用指令
			{
				NextJMCALL = TRUE;
				code->IsCallType = TRUE;
			}
		}
	}
}

// 是否是跳转指令
BOOL CCodeILFactory::IsJumpCmd(t_disasm* da)
{
	if(  ( da->cmdtype & C_CAL ) == C_CAL )//CALL不算
	{
		return FALSE;
	}
	if( ( da->cmdtype & C_JMP ) == C_JMP )
	{
		return TRUE;
	}
	if( ( da->cmdtype & C_JMC ) == C_JMC )
	{
		return TRUE;
	}
	return FALSE;
}
// 是否是函数调用指令
BOOL CCodeILFactory::IsCallCmd(t_disasm* da)
{
	if( ( da->cmdtype & C_CAL ) == C_CAL )
	{
		return TRUE;
	}
	return FALSE;
}
BOOL CCodeILFactory::IsRetnCmd(t_disasm* da)
{
	char strretn[32] = "RETN\0";
	_strlwr_s(da->result,TEXTLEN);
	_strlwr_s(strretn,32);
	if( strstr(strretn,da->result) )
	{
		return TRUE;
	}
	return FALSE;
}
//从链表中搜索一个地址的节点
CodeNode* CCodeILFactory::SearchAddrAsList( list<CodeNode*> *List,CodeNode* node )
{
	DWORD GotoAddr = node->disasm.jmpconst;
	DWORD LastAddr = 0;
	list<CodeNode*>::iterator itr;
	for( itr = List->begin(); itr != List->end(); itr++ )
	{
		CodeNode* code = *itr;
		t_disasm t_da ;
		memcpy(&t_da,&code->disasm,sizeof(t_disasm));
		if( GotoAddr == t_da.ip )//找到了
		{
			node->IsJmcType = TRUE;//正常跳转
			return code;
		}
		else if( GotoAddr > t_da.ip )
		{
			LastAddr = t_da.ip;
		}
		else if( GotoAddr < t_da.ip )
		{
			if( LastAddr )
			{
				node->IsJmcUndefineType = TRUE;//跳到了不明确的地址
			}
		}
	}
	return NULL;
}

// 编译汇编代码为VM字节码,返回成功失败
//BOOL CCodeILFactory::BuildCode(CLink* CodeList,char* ErrText)
BOOL CCodeILFactory::BuildCode(char* baseaddr,list<CodeNode*>* CodeList,char* ErrText)
{
	return VMFactory.BuildVMCode(baseaddr,CodeList,ErrText);
}
 