/*
** zcc_exprlist.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2016 Marisa Heit
** Copyright 2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

// Name				Token used in the code generator
xx(Nil,				TK_None)

xx(ID,				TK_Identifier)
xx(Super,			TK_Super)
xx(Null,			TK_Null)
xx(ConstValue,		TK_Const)
xx(FuncCall,		'(')
xx(ArrayAccess,		TK_Array)
xx(MemberAccess,	'.')
xx(ClassCast,		TK_Class)
xx(FunctionPtrCast,	TK_FunctionType)
xx(TypeRef,			TK_Class)
xx(Vector,			TK_Vector2)

xx(PostInc,			TK_Incr)
xx(PostDec,			TK_Decr)

xx(PreInc,			TK_Incr)
xx(PreDec,			TK_Decr)
xx(Negate,			'-')
xx(AntiNegate,		'+')
xx(BitNot,			'~')
xx(BoolNot,			'!')
xx(SizeOf,			TK_SizeOf)
xx(AlignOf,			TK_AlignOf)

xx(Add,				'+')
xx(Sub,				'-')
xx(Mul,				'*')
xx(Div,				'/')
xx(Mod,				'%')
xx(Pow,				TK_MulMul)
xx(CrossProduct,	TK_Cross)
xx(DotProduct,		TK_Dot)
xx(LeftShift,		TK_LShift)
xx(RightShift,		TK_RShift)
xx(URightShift,		TK_URShift)
xx(Concat,			TK_DotDot)

xx(LT,				'<')
xx(LTEQ,			TK_Leq)
xx(GT,				'>')
xx(GTEQ,			TK_Geq)
xx(LTGTEQ,			TK_LtGtEq)
xx(LTEQGT,			TK_LtEqGt)
xx(Is,				TK_Is)

xx(EQEQ,			TK_Eq)
xx(NEQ,				TK_Neq)
xx(APREQ,			TK_ApproxEq)

xx(BitAnd,			'&')
xx(BitOr,			'|')
xx(BitXor,			'^')
xx(BoolAnd,			TK_AndAnd)
xx(BoolOr,			TK_OrOr)

xx(Assign,			'=')
xx(AddAssign,		'+')	// these are what the code generator needs, not what they represent.
xx(SubAssign,		'-')
xx(MulAssign,		'*')
xx(DivAssign,		'/')
xx(ModAssign,		'%')
xx(LshAssign,		TK_LShift)
xx(RshAssign,		TK_RShift)
xx(URshAssign,		TK_URShift)
xx(AndAssign,		'&')
xx(OrAssign,		'|')
xx(XorAssign,		'^')

xx(Scope,			TK_ColonColon)

xx(Trinary,			'?')

xx(Cast,			TK_Coerce)

#undef xx
