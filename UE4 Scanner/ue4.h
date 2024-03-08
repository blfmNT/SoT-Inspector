#pragma once
#include "common.h"
#include "utils.h"

using namespace MEMORY;

namespace UE4
{
    template< class T > struct TArray
    {
    public:
        T* Data;
        int Count;
        int Max;

    public:
        TArray()
        {
            Data = NULL;
            Count = Max = 0;
        };

    public:
        int Num()
        {
            return this->Count;
        };

        T& operator() (int i)
        {
            return this->Data[i];
        };

        const T& operator() (int i) const
        {
            return this->Data[i];
        };

        void Add(T InputData)
        {
            Data = (T*)realloc(Data, sizeof(T) * (Count + 1));
            Data[Count++] = InputData;
            Max = Count;
        };

        void Clear()
        {
            free(Data);
            Count = Max = 0;
        };
    };

    struct FName
    {
        int				Index;
        unsigned char	unknownData00[0x4];
    };

    struct FString : public TArray< wchar_t > {

    };
    class FUObjectItem
    {
    public:
        ULONG_PTR Object; //0x0000
        __int32 Flags; //0x0008
        __int32 ClusterIndex; //0x000C
        __int32 SerialNumber; //0x0010
        __int32 pad; //0x0010
    };

    class TUObjectArray
    {
    public:
        FUObjectItem* Objects;
        int32_t MaxElements;
        int32_t NumElements;
    };

    class FUObjectArray
    {
    public:
        __int32 ObjFirstGCIndex; //0x0000
        __int32 ObjLastNonGCIndex; //0x0004
        __int32 MaxObjectsNotConsideredByGC; //0x0008
        __int32 OpenForDisregardForGC; //0x000C

        TUObjectArray ObjObjects; //0x0010
    };
    class CObjects {

    public:
        static int32_t GetCount() {
            auto ptr = Read<int32_t>((PBYTE)GObjects + offsetof(TUObjectArray, NumElements));
            return ptr;
        }
        static ULONG_PTR GetObject(int id) {
            // auto ptr = Read<ULONG_PTR>((PBYTE)GObjects + offsetof(FUObjectArray, ObjObjects));
            return Read<ULONG_PTR>(((PBYTE)GObjects + (id * sizeof(FUObjectItem))));
        }
    };

    class CNames {
    public:
        static int GetChunkCount() {
            return Read<int>((PBYTE)GNames + 0x404);
        }
        static const char* GetName(int id) {
            static char m_name[124];
            auto pData = Read<ULONG_PTR>((PBYTE)GNames + int(id / 0x4000) * 8);
            LPBYTE pEntry = Read<LPBYTE>(((PBYTE)(pData + 8 * int(id % 0x4000))));
            ZeroMemory(m_name, sizeof(m_name));
            ReadProcessMemory(hProcess, (LPCVOID)(pEntry + 0x10), (uint8_t*)m_name, sizeof(m_name) - 2, NULL) != 0;
            return m_name;
        }
    };
    class AActor {
    public:
        ULONG_PTR _this;
        AActor(ULONG_PTR ptr) : _this(ptr) {
        }
        int GetId() {
            return Read<int>((LPBYTE)_this + 0x18);
        }
        const char* GetName() {
            return CNames::GetName(GetId());//"name";
        }
    };

#pragma pack(push,1)
    struct encryptedActor
    {
        uint64_t ptr_table[0x2B];
        uint16_t index;
        byte  unk2[0x6];
        uint16_t xor;
        byte  unk3[0x6];
    };
#pragma pack(pop)
    class CWorld : public AActor {
    public:
        CWorld(ULONG_PTR ptr) : AActor(ptr) {
        }
        std::vector<AActor> GetActors() {
            std::vector<AActor> v;
            TArray<ULONG_PTR> levels = Read<TArray<ULONG_PTR>>((LPBYTE)_this + 0x150);
            for (int i = 0; i < levels.Count; i++) {
                ULONG_PTR ptr = Read<ULONG_PTR>((LPBYTE)levels.Data + (i * 8));
                if (ptr) {
                    TArray<ULONG_PTR> buf = Read<TArray<ULONG_PTR>>((LPBYTE)ptr + 0xA0);
                    for (int i = 0; i < buf.Count; i++) {
                        ULONG_PTR ptr2 = Read<ULONG_PTR>((LPBYTE)buf.Data + (i * 8));
                        if (ptr2)
                            v.push_back(AActor(ptr2));
                    }
                }
            }
            //get level and list actors
            return v;
        };
    };
    struct FPointer
    {
        ULONG_PTR Dummy;
    };
    struct FQWord
    {
        int A;
        int B;
    };
    class UClass;
    class UObject
    {
    public:
        FPointer VTableObject;
        int32_t ObjectFlags;
        int32_t InternalIndex;
        UClass* Class;
        FName Name;
        UObject* Outer;
    };
    class UField : public UObject
    {
    public:
        UField* Next;
    };
    template<typename KeyType, typename ValueType>
    class TPair
    {
    public:
        KeyType   Key;
        ValueType Value;
    };

    class UEnum : public UField
    {
    public:
        FString CppType; //0x0030 
        TArray<TPair<FName, uint32_t>> Names; //0x0040
        __int64 CppForm; //0x0050
        char pad_0x0058[0x8]; //0x0058
    };

    class UStruct : public UField
    {
    public:
        UStruct* SuperField;
        UField* Children;
        int PropertySize;
        int MinAlignment;
        TArray<uint8_t> Script;
        class UProperty* PropertyLink;
        class UProperty* RefLink;
        class UProperty* DestructorLink;
        class UProperty* PostConstructLink;
        TArray<UObject*> ScriptObjectReferences;
    };
    class UProperty : public UField
    {
        using UField::UField;
    public:
        int ArrayDim;
        int ElementSize;
        uint64_t PropertyFlags;
        char pad[0xC];
        int Offset_Internal;
        UProperty* PropertyLinkNext;
        UProperty* NextRef;
        UProperty* DestructorLinkNext;
        UProperty* PostConstructLinkNext;
    };
    class UFunction : public UStruct
    {
    public:
        int FunctionFlags;
        uint16_t RepOffset;
        uint8_t NumParms;
        char pad;
        uint16_t ParmsSize;
        uint16_t ReturnValueOffset;
        uint16_t RPCId;
        uint16_t RPCResponseId;
        UProperty* FirstPropertyToInit;
        UFunction* EventGraphFunction; //0x00A0
        int EventGraphCallOffset;
        char pad_0x00AC[0x4]; //0x00AC
        void* Func; //0x00B0
    };
    class UScriptStruct : public UStruct
    {
    public:
        char pad_0x0088[0x10]; //0x0088
    };

    class UClass : public UStruct
    {
    public:
        unsigned char UnknownData00[0x138];                                     // 0x0088(0x0138) MISSED OFFSET
    };

    class UBoolProperty : public UProperty
    {
    public:
        unsigned long		BitMask;									// 0x0088 (0x04)
    };
    class UArrayProperty : public UProperty
    {
    public:
        UProperty* Inner;
    };
    class UMapProperty : public UProperty
    {
    public:
        UProperty* KeyProp;
        UProperty* ValueProp;
    };
    class UStructProperty : public UProperty
    {
    public:
        UScriptStruct* Struct;
    };
    template<class T>
    class UProxy {
    public:
        ULONG_PTR ptr;
        T obj;
        UProxy(ULONG_PTR _ptr) : ptr(_ptr) {
            ReadTo((LPBYTE)_ptr, &obj, sizeof(obj));
        }
        T* GetObject() {
            return &obj;
        }
        int GetId() {
            return obj.Name.Index;
        }
        std::string GetName() {
            return CNames::GetName(GetId());
        }
        bool IsA(UClass* pClass)
        {
            /*for (UClass* SuperClass = this->Class; SuperClass; SuperClass = (UClass*)SuperClass->SuperField)
            {
                if (SuperClass == pClass)
                    return true;
            }*/

            return false;
        }
        template <class T>
        T As() {
            return T(ptr);
        }
        UProxy GetClass() {
            return UProxy((ULONG_PTR)obj.Class);
        }
        bool HasOuter() {
            return obj.Outer != NULL;
        }
        UProxy GetOuter() {
            return UProxy((ULONG_PTR)obj.Outer);
        }
        virtual bool Is(std::string name) {

            return GetClass().GetName() == name;
        }
        bool IsMulticastDelegate() { return Is("MulticastDelegateProperty"); }
        bool IsFunction() { return Is("Function"); }
        bool IsStruct() { return Is("StructProperty"); }
        bool IsFloat() { return Is("FloatProperty"); }
        bool IsBool() { return Is("BoolProperty"); }
        bool IsName() { return Is("NameProperty"); }
        bool IsByte() { return Is("ByteProperty"); }
        bool IsWeakObject() { return Is("WeakObjectProperty"); }
        bool IsObject() { return Is("ObjectProperty") || IsWeakObject(); }
        bool IsInt() { return Is("IntProperty"); }
        bool IsInt8() { return Is("Int8Property"); }
        bool IsUIn32() { return Is("UInt32Property"); }
        bool IsUInt64() { return Is("UInt64Property"); }
        bool IsClass() { return Is("ClassProperty") || Is("Class"); }
        bool IsArray() { return Is("ArrayProperty"); }
        bool IsMap() { return Is("MapProperty"); }
        bool IsString() { return Is("StrProperty"); }
        bool IsField() { return Is("Field"); }
        bool IsWidget() { return Is("UserWidget"); }
        bool IsProperty() { return Is("Property") || IsArray() || IsInt() || IsObject() || IsWeakObject() || IsByte() || IsName() || IsBool() || IsFloat(); }
        bool IsPackage() {
            return Is("Package");
        }
        bool IsIgnore() {
            return strstr(GetName().c_str(), "Default__") || IsPackage() || IsClass() || IsFunction() || IsStruct() || IsProperty() || IsWidget();
        }
        char* GetFullName()
        {
            if (obj.Class && obj.Outer)
            {
                static char cOutBuffer[512];

                char cTmpBuffer[512];

                strcpy_s(cOutBuffer, this->GetName().c_str());

                for (UProxy pOuter = this->GetOuter(); 1; pOuter = pOuter.GetOuter())
                {
                    strcpy_s(cTmpBuffer, pOuter.GetName().c_str());
                    strcat_s(cTmpBuffer, ".");

                    size_t len1 = strlen(cTmpBuffer);
                    size_t len2 = strlen(cOutBuffer);

                    memmove(cOutBuffer + len1, cOutBuffer, len1 + len2 + 1);
                    memcpy(cOutBuffer, cTmpBuffer, len1);
                    if (!pOuter.HasOuter())
                        break;
                }

                strcpy_s(cTmpBuffer, this->GetClass().GetName().c_str());
                strcat_s(cTmpBuffer, " ");

                size_t len1 = strlen(cTmpBuffer);
                size_t len2 = strlen(cOutBuffer);

                memmove(cOutBuffer + len1, cOutBuffer, len1 + len2 + 1);
                memcpy(cOutBuffer, cTmpBuffer, len1);

                return cOutBuffer;
            }

            return "(null)";
        }
        bool HasChildren() {
            return obj.Children != NULL;
        }
        UProxy GetChildren() {
            return UProxy((ULONG_PTR)obj.Children);//);Read<ULONG_PTR>((LPBYTE)ptr+offsetof(UStruct,Children)));//(ULONG_PTR)obj.Children);
        }
    };
    class UFieldProxy : public UProxy<UField> {
    public:
        UFieldProxy(ULONG_PTR _ptr) : UProxy<UField>(_ptr) {

        }
    };
    class UPropertyProxy : public UProxy<UProperty> {
    public:
        UPropertyProxy(ULONG_PTR _ptr) : UProxy<UProperty>(_ptr) {

        }
        bool HasNext() {
            char msg[124];
            sprintf_s(msg, 124, "%p\n", obj.Next);
            //OutputDebugStringA(msg);
            return obj.Next != NULL && (ULONG_PTR)obj.Next != 0xCCCCCCCCCCCCCCCC && (ULONG_PTR)obj.Next != 0;
        }
        UPropertyProxy GetNext() {
            return UPropertyProxy((ULONG_PTR)obj.Next);
        }
        int GetOffset() {
            return obj.Offset_Internal;
        }
        unsigned long GetBitMask() {
            return Read<unsigned long>((LPBYTE)ptr + offsetof(UBoolProperty, BitMask));
        }
        UPropertyProxy GetInner() {
            return UPropertyProxy(Read<ULONG_PTR>((LPBYTE)ptr + offsetof(UArrayProperty, Inner)));
        }
        UPropertyProxy GetKey() {
            return UPropertyProxy(Read<ULONG_PTR>((LPBYTE)ptr + offsetof(UMapProperty, KeyProp)));
        }
        UPropertyProxy GetValue() {
            return UPropertyProxy(Read<ULONG_PTR>((LPBYTE)ptr + offsetof(UMapProperty, ValueProp)));
        }
        UProxy GetStruct() {
            UProxy p(Read<ULONG_PTR>((LPBYTE)ptr + offsetof(UStructProperty, Struct)));
            return p;
        }
        int GetArrayDim() {
            return obj.ArrayDim;
        }
        int GetElementSize() {
            return obj.ElementSize;
        }
        int GetSize() {
            return GetArrayDim() * GetElementSize();
        }
    };
    class UClassProxy : public UProxy<UClass> {
    public:
        UClassProxy(ULONG_PTR _ptr) : UProxy<UClass>(_ptr) {

        }
        int GetSize() {
            return obj.PropertySize;
        }
        bool HasSuperClass() {
            return obj.SuperField != NULL;
        }
        UClassProxy GetSuperClass() {
            return UClassProxy((ULONG_PTR)obj.SuperField);
        }
        std::string GetFullClass() {
            std::string str;

            auto c = *this;
            while (c.HasSuperClass()) {
                std::string className = c.GetName();
                if (className.empty())
                    break;
                str.append(".").append(className);
                c = c.GetSuperClass();
            }
            return str;
        }
        virtual bool Is(std::string name) {
            auto c = *this;
            while (c.HasSuperClass()) {
                if (c.GetName() == name)
                    return true;
                c = c.GetSuperClass();
            }
            return c.GetName() == name;
        }
    };
    class UObjectProxy : public UProxy<UObject> {
    public:
        UObjectProxy(ULONG_PTR _ptr) : UProxy<UObject>(_ptr) {

        }
        virtual bool Is(std::string name) {
            return GetClass().Is(name);
        }
    };
}