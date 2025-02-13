/**
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
*/

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <ctime>
#include <x86_64-linux-gnu/curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// GLOBAL //
int Count[46];					// 45개의 숫자들이 최근 15번동안 나온 횟수
std::string response_data;		// 응답 데이터를 저장할 변수

// FUNCTION //
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);		// cURL의 WriteCallback 함수 (응답 데이터를 저장)
int GETRecentDRWTNO();																		// 저장된 파일에서 최근 Lotto 회차를 가져오는 함수
void ChangeRecentDRWTNO(int n);																// 파일에 저장된 데이터를 가장 최신의 Lotto 회차로 바꾸는 함수
int CheckRecentDRWTNO(int n);																// 최근 회차인지 체크하고, 최신 회차를 return 하는 함수
bool GETLotto(int n);																		// 최근 15회차 Lotto에 나온 번호를 조회하는 함수

// MAIN //
int main()
{
	// key : 등장 횟수
	// value : 번호
	std::multimap<int, int> m;
	std::multimap<int, int>::iterator it;

	int arr[7] = {0,};
	bool IsChoose[46] = {false,};
	int recent, idx, tmp;

	srand(time(NULL));

	// 저장된 데이터를 가져온 후, 최신 데이터인지 검사
	recent = GETRecentDRWTNO();
	tmp = CheckRecentDRWTNO(recent);

	// API 요청 실패
	if(!tmp)
	{
		std::cout << "체크 실패" << std::endl;
		return 0;
	}

	// 최신 데이터로 갱신
	if(tmp != recent)
	{
		recent = tmp;
		ChangeRecentDRWTNO(recent);
	}

	// 최근 데이터 가져오기
	if(!GETLotto(recent))
	{
		std::cout << "최근 데이터 가져오기 실패" << std::endl;
		return 0;
	}

	// 최근 등장 횟수가 작은 순으로 정렬해 저장
	for(int i=1; i<46; i++)
		m.insert(std::make_pair(Count[i], i));

	// 처음 3개의 숫자는 잘 나오지 않은 순으로 추천
	idx = 0;
	it = m.begin();
	while(1)
	{
		if(idx > 3)
			break;

		if(!IsChoose[it->second])
		{
			arr[idx] = it->second;
			IsChoose[it->second] = true;

			idx++;
			it++;
		}
	}

	// 마지막 3개의 숫자는 잘 나오는 순으로 추천
	it = m.end();
	it--;
	while(1)
	{
		if(idx > 7)
			break;

		if(!IsChoose[it->second])
		{
			arr[idx] = it->second;
			IsChoose[it->second] = true;

			idx++;
			it--;
		}
	}

	// 보너스 번호는 아무거나 추천
	while(1)
	{
		tmp = rand() % 46;

		if(tmp && !IsChoose[tmp])
		{
			arr[idx] = tmp;
			break;
		}
	}

	// 최신 회차 출력
	std::cout << "---------------------------" << std::endl;
	std::cout << "최신 회차 : " << recent << std::endl;
	std::cout << "---------------------------" << std::endl;

	// 뽑힌 숫자들을 정렬해 출력
	std::sort(arr, arr+7);

	std::cout << "1. 뽑힌 숫자들 : ";
	for(int i=0; i<7; i++)
	{
		std::cout << arr[i];
		if(i != 6)
			std::cout << ", ";
		else
			std::cout << std::endl;
	}

	std::cout << "---------------------------" << std::endl;

	// 최근 15 회차에서 각 숫자들이 나온 횟수 출력
	std::cout << "2. 최근 15회간 숫자들이 나온 횟수(숫자 : 나온 횟수)" << std::endl;
	for(int i=1; i<46; i++)
	{
		std::cout << i;

		if(!(i/10))
			std::cout << " : ";
		else
			std::cout << ": ";

		std::cout << Count[i] << "번" << std::endl;
	}
		

	return 0;
}


// cURL의 WriteCallback 함수 (응답 데이터를 저장)
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// 저장된 파일에서 최근 Lotto 회차를 가져오는 함수
int GETRecentDRWTNO()
{
	std::ifstream in("recent.txt");
	std::string s;
	int n = 0;

	if(in.is_open())
	{
		in >> s;
		n = std::stoi(s);
		std::cout << "저장된 최근 회차 : " << n << std::endl;
	}
	else
	{
		std::cout << "파일이 없습니다." << std::endl;
	}

	in.close();

	return n;
}

// 파일에 저장된 데이터를 가장 최신의 Lotto 회차로 바꾸는 함수
void ChangeRecentDRWTNO(int n)
{
	std::ofstream out("recent.txt");

	if(out.is_open())
    {
        out << n;
        std::cout << "저장한  최근 회차 : " << n << std::endl;
    }
    else
    {
        std::cout << "파일이 없습니다." << std::endl;
    }

    out.close();
}

// 최근 회차인지 체크하고, 최신 회차를 return 하는 함수
int CheckRecentDRWTNO(int n)
{
	CURL* curl;
    CURLcode res;
    std::string url = "https://www.dhlottery.co.kr/common.do?method=getLottoNumber&drwNo=";
    const size_t len = url.length();
    int prev = n - 1;
    std::string tmp = std::to_string(n);
    std::string field = "returnValue";

	// cURL 생성
    curl = curl_easy_init();

    if (curl)
    {
		// cURL 설정
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
		// User-Agent 설정
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");

        while(1)
        {
			// url에 저장된 회차 정보 및 데이터 삭제
            url.erase(len, tmp.length());
			response_data.clear();

			// 회차 정보 갱신 및 url에 삽입
			prev++;
            tmp = std::to_string(prev);
            url += tmp;

			// url 설정 및 GET 요청
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            res = curl_easy_perform(curl);

			// GET 요청 실패
            if (res != CURLE_OK)
            {
                std::cerr << "cURL 요청 실패: " << curl_easy_strerror(res) << std::endl;
				std::cerr << "요청한 url : " << url << std::endl;
                curl_easy_cleanup(curl);
                return 0;
            }

			// JSON 파싱
            try
            {
                json parsed_json = json::parse(response_data);

				// 만약 실패한 경우, 이전에 갱신한 데이터가 최신이므로
				// 이전에 갱신한 데이터 return
                if(parsed_json[field] == "fail")
				{
					curl_easy_cleanup(curl);
					return prev-1;
				}
            }
			// JSON 파싱 오류
            catch (json::parse_error& e)
            {
                std::cerr << "JSON 파싱 오류: " << e.what() << std::endl;
                curl_easy_cleanup(curl);
                return false;
            }
        }
    }

	// curl 핸들 정리
    curl_easy_cleanup(curl);
    return true;
}

// 최근 15회차 Lotto에 나온 번호를 조회하는 함수
bool GETLotto(int n)
{
	CURL* curl;
    CURLcode res;
	std::string url = "https://www.dhlottery.co.kr/common.do?method=getLottoNumber&drwNo=";
	const size_t len = url.length();
	int prev = n + 1;
	std::string tmp = std::to_string(n);
	std::string field;
	std::string drwtNo = "drwtNo";

	// cURL 생성
    curl = curl_easy_init();

	if (curl)
	{
		// cURL 설정
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
		// User-Agent 설정
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");

		for(int i=0; i<15; i++)
		{
			// url에 저장된 Lotto 회차 정보 및 데이터 삭제
			url.erase(len, tmp.length());
			response_data.clear();

			// Lotto 회차 정보 갱신 및 url에 삽입
			prev--;
			tmp = std::to_string(prev);
			url += tmp;

			// url 설정 및 GET 요청
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        	res = curl_easy_perform(curl);

			// GET 요청 실패
        	if (res != CURLE_OK)
			{
            	std::cerr << "cURL 요청 실패: " << curl_easy_strerror(res) << std::endl;
				std::cerr << "요청한 url : " << url << std::endl;
				curl_easy_cleanup(curl);
            	return false;
        	}

        	// JSON 파싱
        	try
			{
            	json parsed_json = json::parse(response_data);

				// 받은 6개의 번호를 index로 하여 1씩 증가
				for(int j=1; j<=6; j++)
				{
					field = drwtNo + std::to_string(j);
					Count[parsed_json[field].get<int>()]++;
				}
            	
				// 보너스 번호도 1 증가
				Count[parsed_json["bnusNo"].get<int>()]++;
        	}
			// JSON 파싱 오류
			catch (json::parse_error& e)
			{
            	std::cerr << "JSON 파싱 오류: " << e.what() << std::endl;
				curl_easy_cleanup(curl);
				return false;
        	}
		}
    }

	// curl 핸들 정리
    curl_easy_cleanup(curl);
	return true;
}