#include <catch2/catch.hpp>
#include "tcppLibrary.hpp"
#include <iostream>


using namespace tcpp;


TEST_CASE("Preprocessor Tests")
{
	auto errorCallback = []()
	{
		REQUIRE(false);
	};

	SECTION("TestProcess_PassSourceWithoutMacros_ReturnsEquaivalentSource")
	{
		std::string inputSource = "void main/* this is a comment*/(/*void*/)\n{\n\treturn/*   */ 42;\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(!preprocessor.Process().empty());
	}

	SECTION("TestProcess_PassSourceWithSimpleMacro_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define VALUE 42\n void main()\n{\n\treturn VALUE;\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		std::cout << preprocessor.Process() << std::endl;
	}

	SECTION("TestProcess_PassSourceWithCorrectFuncMacro_ReturnsSourceWithExpandedMacro")
	{
		std::string inputSource = "#define ADD(X, Y) X + Y\n void main()\n{\n\treturn ADD(2, 3);\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		std::cout << preprocessor.Process() << std::endl;
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsSourceStringWithIncludeDirective")
	{
		std::string inputSource = "#include <system>\n#include \"non_system_path\"\n void main()\n{\n\treturn ADD(2, 3);\n}";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		const std::tuple<std::string, bool> expectedPaths[]{ { "system", true }, { "non_system_path", false } };
		short currExpectedPathIndex = 0;

		Preprocessor preprocessor(lexer, errorCallback, [&input, &currExpectedPathIndex, &expectedPaths](const std::string& path, bool isSystem)
		{
			auto expectedResultPair = expectedPaths[currExpectedPathIndex++];

			REQUIRE(path == std::get<std::string>(expectedResultPair));
			REQUIRE(isSystem == std::get<bool>(expectedResultPair));
			return &input;
		});
		preprocessor.Process();
	}

	SECTION("TestProcess_PassSourceWithIncludeDirective_ReturnsSourceStringWithIncludeDirective")
	{
		std::string inputSource = "__LINE__\n__LINE__\n__LINE__";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "1\n2\n3");
	}

	SECTION("TestProcess_PassSourceWithStringizeOperator_ReturnsSourceWithStringifiedToken")
	{
		std::string inputSource = "#define FOO(Name) #Name\n FOO(Text)";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == " Text");
	}

	/*SECTION("TestProcess_PassSourceWithConcatenationOperator_ReturnsSourceWithConcatenatedTokens")
	{
		std::string inputSource = "#define CAT(X, Y) X ## Y\n CAT(4, 2)";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == " 42");
	}*/

	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutThisBlock")
	{
		std::string inputSource = "#if FOO\n // this block will be skiped\n #endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "");
	}

	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutIfBlock")
	{
		std::string inputSource = "#if FOO\n // this block will be skiped\n if block\n#else\n else block #endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "\n else block ");
	}
	
	SECTION("TestProcess_PassSourceWithConditionalBlocks_ReturnsSourceWithoutElseBlock")
	{
		std::string inputSource = "#if 1\n if block\n#else\n else block #endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == " if block\n");
	}

	SECTION("TestProcess_PassSourceWithElifBlocks_ReturnsSourceWithElabledElifBlock")
	{
		std::string inputSource = "#if 0\none\n#elif 1\ntwo\n#else\nthree\n#endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "two\n");
	}

	SECTION("TestProcess_PassSourceWithFewElifBlocks_ReturnsSourceWithElabledElifBlock")
	{
		std::string inputSource = "#if 0\none\n#elif 0\ntwo\n#elif 1\nthree\n#else\nfour\n#endif";
		StringInputStream input(inputSource);
		Lexer lexer(input);

		Preprocessor preprocessor(lexer, errorCallback);
		REQUIRE(preprocessor.Process() == "three\n");
	}
}