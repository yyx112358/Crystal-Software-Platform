// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� NRIQABRISQUEPYTHONMODULE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// NRIQABRISQUEPYTHONMODULE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef NRIQABRISQUEPYTHONMODULE_EXPORTS
#define NRIQABRISQUEPYTHONMODULE_API __declspec(dllexport)
#else
#define NRIQABRISQUEPYTHONMODULE_API __declspec(dllimport)
#endif

