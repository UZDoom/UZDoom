/*
** namedef.h
**
** common names
**
**---------------------------------------------------------------------------
**
** Copyright 1999-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
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

// clang-format off

// 'None' must always be the first name.
xx(None)
xx(Null)
xx(_)

xx(Super)
xx(Object)
xx(Actor)
xx(Class)
xx(Thinker)
xx(VisualThinker)
xx(Crosshairs)

xx(Untranslated)

// Render styles
xx(Normal)
xx(SoulTrans)
xx(OptFuzzy)
xx(Add)
xx(Shaded)
xx(AddShaded)
xx(TranslucentStencil)
xx(Shadow)
xx(Subtract)
xx(Subtractive)
xx(FillColor)
xx(ColorBlend)
xx(ColorAdd)
xx(Multiply)

// Special names for compiler backend
xx(Name)
xx(Clamp)
xx(Abs)
xx(Random)
xx(FRandom)
xx(Random2)
xx(RandomPick)
xx(FRandomPick)
xx(SetRandomSeed)
xx(CRandom)
xx(CFRandom)
xx(CRandom2)
xx(CRandomPick)
xx(CFRandomPick)
xx(CSetRandomSeed)
xx(BuiltinRandomSeed)
xx(BuiltinNew)
xx(GetClass)
xx(GetParentClass)
xx(GetClassName)
xx(IsAbstract)
xx(GetDefaultByType)
xx(Floor)
xx(Exp)
xx(Log)
xx(Log10)
xx(Ceil)
xx(ACos)
xx(ASin)
xx(ATan)
xx(Cos)
xx(Sin)
xx(Tan)
xx(CosH)
xx(SinH)
xx(TanH)
xx(Round)
xx(ATan2)
xx(VectorAngle)
xx(Sqrt)
xx(New)

xx(Static)
xx(Staticconst)

// compatibility crap that GZDoom needs in the backend.
xx(ACS_NamedExecuteWithResult)
xx(__decorate_internal_int__)
xx(__decorate_internal_bool__)
xx(__decorate_internal_float__)

// Per-actor sound channels (for deprecated PlaySoundEx function) Do not separate this block!!!
xx(Auto)
xx(Weapon)
xx(Voice)
xx(Item)
xx(Body)
xx(SoundSlot5)
xx(SoundSlot6)
xx(SoundSlot7)

xx(LevelLocals)
xx(Level)

xy(menu_cursor, "menu/cursor")
xy(menu_choose, "menu/choose")
xy(menu_backup, "menu/backup")
xy(menu_clear, "menu/clear")
xy(menu_dismiss, "menu/dismiss")
xy(menu_change, "menu/change")
xy(menu_advance, "menu/advance")

// basic type names
xx(Default)
xx(sByte)
xx(Byte)
xx(Short)
xx(uShort)
xx(Int)
xx(uInt)
xx(Bool)
xx(uint8)
xx(int8)
xx(uint16)
xx(int16)
xx(Float)
xx(Float32)
xx(Float64)
xx(Double)
xx(String)
xx(Vector)
xx(Map)
xx(MapIterator)
xx(Array)
xx(Function)
xx(Include)
xx(Sound)
xx(State)
xx(Fixed)
xx(Vector2)
xx(Vector3)
xx(Vector4)
xx(Quat)
xx(FVector2)
xx(FVector3)
xx(FVector4)
xx(FQuat)
xx(let)
xx(BlockThingsIterator)
xx(BlockLinesIterator)
xx(ActorIterator)
xx(ThinkerIterator)

xx(Min)
xx(Max)
xx(Min_Normal)
xx(Min_Denormal)
xx(Epsilon)
xx(Equal_Epsilon)
xx(NaN)
xx(Infinity)
xx(Dig)
xx(Min_Exp)
xx(Max_Exp)
xx(Mant_Dig)
xx(Min_10_Exp)
xx(Max_10_Exp)

// implicit function parameters
xx(self)
xx(invoker)
xx(stateinfo)

xx(DamageFunction)
xx(Length)
xx(LengthSquared)
xx(Sum)
xx(Unit)
xx(Angle)
xx(PlusZ)
xx(Conjugate)
xx(Inverse)
xx(ToVector)
xx(Size)
xx(Push)
xx(Insert)
xx(InsertNew)
xx(Remove)
xx(Get)
xx(GetIfExists)
xx(GetValue)
xx(GetKey)
xx(SetValue)
xx(CheckKey)
xx(CheckValue)
xx(Value)
xx(Copy)
xx(Move)
xx(Voidptr)
xx(StateLabel)
xx(SpriteID)
xx(TextureID)
xx(TranslationID)
xx(Overlay)
xx(IsValid)
xx(IsNull)
xx(IsEmpty)
xx(IsFixed)
xx(IsKeep)
xx(Exists)
xx(SetInvalid)
xx(SetNull)
xx(SetEmpty)
xx(SetFixed)
xx(SetKeep)
xx(Key)
xx(Index)
xx(Find)
xx(Call)

// color channels
xx(a)
xx(r)
xx(g)
xx(b)

xx(X)
xx(Y)
xx(Z)
xx(W)
xx(XY)
xx(XYZ)

xx(Prototype)
xx(Void)
xx(Label)
xx(Pointer)
xx(Enum)
xx(StaticArray)
xx(DynArray)
xx(Struct)
xx(ReflectType)
xx(MessageBoxMenu)

xx(Both)
xx(Physical)
xx(Visual)

// blacklisted former CVARs (used by common menu code)
xx(snd_waterlp)
xx(snd_output)
xx(snd_output_format)
xx(snd_speakermode)
xx(snd_resampler)
xx(AlwaysRun)

// menu names
xx(MainMenu)
xx(EpisodeMenu)
xx(SkillMenu)
xx(Startgame)
xx(StartgameConfirm)
xx(StartgameConfirmed)
xx(LoadgameMenu)
xx(SavegameMenu)
xx(OptionsMenu)
xx(OptionsMenuSimple)
xx(OptionsMenuFull)
xx(QuitMenu)
xx(SaveMenu)
xx(EndGameMenu)
xx(HelpMenu)
xx(SoundMenu)
xx(ConfirmPlayerReset)
xx(JoystickOptions)

xx(OptionMenuItemSubmenu)
xx(OptionMenuItemCommand)
xx(OptionMenuItemControlBase)
xx(OptionMenuItemOptionBase)
xx(OptionMenuSliderBase)
xx(OptionMenuFieldBase)
xx(OptionMenuItemColorPicker)
xx(OptionMenuItemStaticText)
xx(OptionMenuItemStaticTextSwitchable)

xx(InternalHandler)

xx(Color)

xx(Mididevices)
xx(Aldevices)
xx(Alresamplers)

// Decorate compatibility functions
xx(BuiltinRandom)
xx(BuiltinRandom2)
xx(BuiltinFRandom)
xx(BuiltinNameToClass)
xx(BuiltinClassCast)
xx(BuiltinFunctionPtrCast)
xx(BuiltinFindTranslation)
xx(HandleDeprecatedFlags)
xx(CheckDeprecatedFlags)

xx(ScreenJobRunner)
xx(Action)

// clang-format on
