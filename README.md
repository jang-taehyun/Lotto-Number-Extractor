# Lotto-Number-Extractor
- 복권 번호를 추출하는 프로그램.

# 동작 방식
- 최근 15회차의 Lotto에서 뽑힌 숫자들 중, 가장 뽑히지 않는 3개의 숫자와 가장 잘뽑힌 3개의 숫자를 추출.
- 보너스 번호는 뽑히지 않는 숫자들 중 랜덤으로 추출.
- 이후, 최근 15회차의 Lotto에서 각 숫자들이 뽑힌 횟수를 출력.

 * <프로젝트명>
 * - REST API와 C++을 이용한 복권 번호 추천 프로그램
 * 
 * <사용한 기술>
 * - C++, REST API(cURL, libcurl), JSON
 * 
 * <Run-time 환경>
 * - Ubuntu 24.04.2 LTS (GNU/Linux 6.8.0-51-generic x86_64)
 * 
 * <설치할 package 및 complie 방법>
 * - cURL 라이브러리 설치 명령어(curl/curl.h)
 * 	 -> apt install libcurl4-openssl-dev -y
 * 
 * - JSON 라이브러리 설치 명령어(nlohmann/json.hpp)
 *   -> apt install nlohmann-json3-dev
 * 
 * - g++ 컴파일 명령어
 *   -> g++ main.cpp -o Lotto -I/usr/include/x86_64-linux-gnu -lcurl
 * 
 * <어려웠던 점>
 * 1. curl/curl.h가 설치되지 않음
 *   1) 문제 상황 : libcurl4-openssl-dev package를 설치해도 컴파일 시에 curl/curl.h가 없다는 에러가 발생
 *   2) 접근 방법
 *     (1) libcurl4-openssl-dev package를 설치하면 /usr/include directory 안 어딘가에 curl/curl.h가 있을 것이라고 생각.
 * 	   (2) 때문에 'dpkg -S curl.h' 명령어를 통해 curl directory의 위치를 찾음.
 *   3) 해결
 *     (1) 결국 /usr/include/x86_64-linux-gnu directory 안에서 curl directory를 찾음.
 *     (2) 컴파일 시에 해당 경로를 인식하도록 x86_64-linux-gnu directory의 path를 추가.
 * 
 * 2. 복권 API의 URL로 GET 요청을 보내면, 요청에 실패함.
 *   1) 문제 상황 : cURL을 통해 복권 API의 URL(https://www.dhlottery.co.kr/common.do?method=getLottoNumber&drwNo=회차번호)로 GET 요청을 보내면,
 *         JSON 데이터 내부의 "returnValue"가 "fail"를 받음.
 *   2) 접근 방법
 *     (1) 어떻게 접근해야 될지 몰라, chatGPT에게 물어봄.
 *     (2) chatGPT는
 * 		    "일부 서버는 브라우저(크롬, 엣지)에서 오는 요청은 허용하지만, curl과 같은 명령줄 도구에서 오는 요청은 차단할 수 있습니다.
 *			 이는 서버가 User-Agent 헤더를 검사하여 curl을 차단하는 경우 발생합니다."
 *		   라고 설명.
 *	   (3) ubuntu 상에서 curl 명령어를 통해 JSON 데이터를 받아오는지 확인해본 결과, 동일한 현상 발생.
 *	   (4) 때문에 curl 명령어를 날릴 때 User-Agent 헤더를 추가하여 명령어를 날린 결과, JSON 데이터가 정상적으로 받아와짐.
 *	       - 명령어 : 'curl -A "Mozilla" "https://www.dhlottery.co.kr/common.do?method=getLottoNumber&drwNo=1"'
 *	 3) 해결
 *     (1) 코드 내에 cURL 핸들을 설정할 때, curl_easy_setopt() 함수를 이용해 User-Agent도 같이 설정.
 * 
 * 3. curl_easy_setopt() 함수에서 url을 제대로 인식하지 못함.
 *   1) 문제 상황 : curl_easy_setopt() 함수를 통해 url을 넣고 GET 요청을 보내면, 'URL using bad/illegal format or missing URL' 에러 발생.
 *   2) 접근 방법
 *     (1) 코드 내에서 만들어진 URL을 std::cout 객체를 통해 프린트 해본 결과, 복권 API의 URL과 동일함.
 *     (2) chrome, microsoft edge 상에서는 해당 URL이 정상적으로 작동하므로, format에 초점을 맞춤.
 *     (3) 구글링 결과, curl_easy_setopt() 함수의 두 번째 parameter에 CURLOPT_URL 옵션을 넣으면, 3번째 parameter는 char* 형식의 데이터를 넣을 수 있음.
 *     (4) 때문에 string을 char* 형식으로 바꿔서 curl_easy_setopt() 함수에 넣어줘야 함.
 *   3) 해결
 *     (1) std::string의 c_str() 함수를 통해 char* 형식으로 바꿔서 전달
 * 
 * 4. 컴파일 시, 'error: no match for ‘operator[]’ (operand types are ‘int [46]’ and ‘nlohmann::json_abi_v3_11_3::basic_json<>::value_type’ {aka ‘nlohmann::json_abi_v3_11_3::basic_json<>’})' 에러 발생
 *   1) 문제 상황 : 컴파일 시에, 해당 오류가 "Count[parsed_json[field]]++;" 코드에서 발생함.
 *   2) 접근 방법
 *     (1) 오류를 해석해보면, 연산자 []에 맞는게 없다는 뜻이므로, parsed_json[field]에서 문제가 발생한 것으로 생각함.
 *     (2) 이후, 오류의 뒷부분을 보면, parsed_json[field]의 자료형은 nlohmann::json
 *     (3) 때문에 parsed_json[field] 안에 있는 int형 데이터를 꺼낼 필요가 있음.
 *   3) getter를 통해(get<>() 템플릿 함수), parsed_json[field] 안에 있는 데이터를 return
 *     -> 코드를 "Count[parsed_json[field].get<int>()]++;"로 수정.
 *   % 주의 사항 %
 *    * nlohmann::json 객체는 std::cout 객체를 통해 데이터를 바로 출력이 가능하도록 오버로딩이 되어 있음.
 *      -> std::cout << "dwrtNo1: " << parsed_json["dwrtNo1"] << std::endl;
