#include "Mastodon.h"

Mastodon::Mastodon(const std::string& HostName, const std::string& TokenStr)
{
	InstanceName = HostName;
	ServerUrl = "https://" + InstanceName;
	if(TokenStr != "")
	{
		token = TokenStr;
	}
	else
	{
		token = "";
	}

	cHandler = NULL;

	TLnext = "";
	TLprev = "";
	FollowingNext = "";
	FollowingPrev = "";
	FollowersNext = "";
	FollowersPrev = "";
	AccountsStatusNext = "";
	AccountsStatusPrev = "";
	BlockListNext = "";
	BlockListPrev = "";
	FavouritesNext = "";
	FavouritesPrev = "";
	FollowRequestNext = "";
	FollowRequestPrev = "";
	MutesNext = "";
	MutesPrev = "";
	NotificationsNext = "";
	NotificationsPrev = "";
	RebloggedNext = "";
	RebloggedPrev = "";
	FavouritedNext = "";
	FavouritedPrev = "";

	std::regex next("max_id=(\\d*)");
	std::regex prev("since_id=(\\d*)");

}

Mastodon::~Mastodon()
{
	if (cHandler)
	{
		curl_easy_cleanup(cHandler);
		cHandler = NULL;
	}
}

void Mastodon::SetInstanceUrl(const std::string& instancename)
{
	InstanceName = instancename;
	ServerUrl = "https://" + InstanceName;
}

// Curl Callback
size_t Mastodon::CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

// mime type check
std::string Mastodon::CheckMediaType(std::ifstream& fs)
{
	std::string result = "";
	unsigned char* bheader = new unsigned char[8];

	fs.clear();
	fs.seekg(0, fs.beg);
	fs.read((char*)bheader, 8);

	for (int i = 0; i < 8; i++)
	{
		unsigned int hex = bheader[i];
		std::stringstream ss;
		ss << std::hex << hex;
		result += ss.str();
	}

	std::regex jpg("ffd8[\\w]*");
	std::regex png("89504e47[\\w]*");
	std::regex gif("474946[\\w*]*");

	if (std::regex_match(result, jpg))
	{
		return "image/jpeg";
	}
	else if (std::regex_match(result, png))
	{
		return "image/png";
	}
	else if (std::regex_match(result, gif))
	{
		return "image/gif";
	}

	return result;
}

// base64 safe url encode
std::string Mastodon::Base64Safe(std::string& str)
{
	std::regex plus("[+]");
	std::regex slash("[/]");
	std::regex pading("[=]");
	std::string tmp = std::regex_replace(str, plus, "-");
	tmp = std::regex_replace(tmp, slash, "_");
	tmp = std::regex_replace(tmp, pading, ".");

	return tmp;
}


// Image to base64 encode func
std::string Mastodon::ImageToBase64(std::ifstream& fs)
{
	unsigned char* buf;
	unsigned int length;

	//std::ifstream readfile(fname.c_str(),std::ios::binary);
	fs.clear();
	fs.seekg(0, fs.end);
	length = (int)fs.tellg();
	fs.clear();
	fs.seekg(0, fs.beg);
	buf = new unsigned char[length];
	fs.read((char*)buf, length);
	std::string decoded = base64_encode(buf, length);

	delete buf;
	return decoded;
}

// Accounts
std::string Mastodon::GetAccount(const int& id)
{

	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = "";

	if (id == 0)
	{
		url += ServerUrl + MSTDNAPI_Strings::GetCurrentUser_String;
	}
	else
	{
		url += ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id);
	}

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::UpdateAccount(const std::string display_name, const std::string note, const std::string avatar, const std::string header)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::UpdatingCurrentUser_String;

	std::string params = "";

	std::string avatarimg = "data:";
	std::string headerimg = "data:";

	if (display_name != "")
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "display_name", CURLFORM_COPYCONTENTS, display_name.c_str(), CURLFORM_END);
	}

	if (note != "")
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "note", CURLFORM_COPYCONTENTS, note.c_str(), CURLFORM_END);
	}

	if (avatar != "")
	{
		std::ifstream fs(avatar.c_str(), std::ios::binary);
		if (fs)
		{
			std::string mediatype = CheckMediaType(fs);
			avatarimg += mediatype + ";base64,";
			avatarimg += ImageToBase64(fs);
			curl_formadd(&post, &last, CURLFORM_COPYNAME, "avatar", CURLFORM_COPYCONTENTS, avatarimg.c_str(), CURLFORM_END);
		}
	}

	if (header != "")
	{
		std::ifstream fs(avatar.c_str(), std::ios::binary);
		if (fs)
		{
			std::string mediatype = CheckMediaType(fs);
			headerimg += mediatype + ";base64,";
			headerimg += ImageToBase64(fs);
			curl_formadd(&post, &last, CURLFORM_COPYNAME, "header", CURLFORM_COPYCONTENTS, headerimg.c_str(), CURLFORM_END);
		}
	}

	std::string setheader = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, setheader.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPPOST, post);
	curl_easy_setopt(cHandler, CURLOPT_CUSTOMREQUEST, "PATCH");
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);
	curl_formfree(post);

	return buf;
}

std::string Mastodon::GetFollowers(const int& id, const int& max_id, const int& since_id, const int& limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::GettingAccountsFollowers_String;
	std::string params = "";


	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 80)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	if (params != "")
	{
		url += "?" + params;
	}

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			FollowersNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{

			FollowersPrev = m[1].str();

		}
	}


	return buf;
}

std::string Mastodon::GetFollowersNext()
{
	return FollowersNext;
}

std::string Mastodon::GetFollowersPrev()
{
	return FollowersPrev;
}

std::string Mastodon::GetFollowing(const int& id, const int& max_id, const int& since_id, const int& limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::GettingAccountsFollowing_String;
	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 80)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	if (params != "")
	{
		url += "?" + params;
	}

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{

			FollowingNext = m[1].str();

		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{

			FollowingPrev = m[1].str();

		}
	}

	return buf;
}

std::string Mastodon::GetFollowingNext()
{
	return FollowersNext;
}

std::string Mastodon::GetFollowingPrev()
{
	return FollowersPrev;
}

std::string Mastodon::GetAccountsStatuses(const int & id, bool only_media, bool exclude_replies, const int & max_id, const int & since_id, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = "";
	std::string params = "";

	url += ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::GettingAccountsFollowers_String;


	if (only_media)
	{
		if (params == "")
		{
			params += "only_media=true";
		}
		else
		{
			params += "&only_media~";
		}
	}


	if (exclude_replies)
	{
		if (params == "")
		{
			params += "exclude_replies=true";
		}
		else
		{
			params += "&exclude_replies=true";
		}
	}

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 80)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	if (params != "")
	{
		url += "?" + params;
	}

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			AccountsStatusNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			AccountsStatusPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetAccountsStatusNext()
{
	return AccountsStatusNext;
}

std::string Mastodon::GetAccountsStatusPrev()
{
	return AccountsStatusPrev;
}

std::string Mastodon::GetAccountsFollow(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::FollowAccount_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetAccountsUnFollow(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::UnFollowingAccount_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetAccountsBlock(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::AccountBlocking_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetAccountsUnBlock(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::AccountUnBlocking_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetAccountsMute(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::AccountMuting_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetAccountsUnMute(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::AccountUnMuting_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetAccountsRelationships(const std::vector<int>& ids)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + MSTDNAPI_Strings::AccountRelationShips;
	std::string params = "?id[]=";
	int count = 0;

	for (int n : ids)
	{
		if (count == 0)
		{
			params += std::to_string(n);
			count += 1;
		}
		else
		{
			params += "&id[]=" + std::to_string(n);
			printf("%d\n", n);
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::AccountsSearch(const std::string & q, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + MSTDNAPI_Strings::AccountSearch_String;
	std::string params = "?q=" + q;

	if (limit < 40 || limit > 40)
	{
		params += "&limit=" + std::to_string(limit);
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

// Apps
std::string Mastodon::CreateApp(const std::string& AppName, const std::vector<std::string>& scopesparam, const std::string& redirect, const std::string& website, const std::string& fname)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::apiBase + MSTDNAPI_Strings::CreateAppAPI_String;
	std::string scopes = "";
	std::string buf;
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "client_name", CURLFORM_COPYCONTENTS, AppName.c_str(), CURLFORM_END);
	for (std::string str : scopesparam)
	{

		scopes += str + " ";
	}
	scopes.erase(scopes.size() - 1);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "scopes", CURLFORM_COPYCONTENTS, scopes.c_str(), CURLFORM_END);

	if (redirect == "")
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "redirect", CURLFORM_COPYCONTENTS, "&redirect_uris=urn:ietf:wg:oauth:2.0:oob", CURLFORM_END);
	}
	else
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "redirect", CURLFORM_COPYCONTENTS, redirect.c_str(), CURLFORM_END);
	}

    if (website != "")
    {
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "website", CURLFORM_COPYCONTENTS, website.c_str(), CURLFORM_END);
    }

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_HTTPPOST, post);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_formfree(post);
	curl_slist_free_all(headerList);

	if (res == 0)
	{

		if(fname != "")
		{
			std::ofstream writefile;
			writefile.open(fname, std::ios::out);
			writefile << buf << std::endl;
		}

	}

	return buf;
}

// Get Token Response
std::string Mastodon::GetToken(const std::string &client_id, const std::string &id_secret, const std::string &Mail, const std::string &Pass, const std::string& fname)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + "/oauth/token";
	std::string post_data = "client_id=" + client_id + "&client_secret" + id_secret + "&scope=read write follow&grant_type=password&username=" + Mail + "&password=" + Pass;
	std::string buf;

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	if (res == CURLM_OK)
	{
		if (fname != "")
		{
			std::ofstream writefile;
			writefile.open(fname, std::ios::out);
			writefile << buf << std::endl;
		}

		picojson::value v;
		std::string err;
		picojson::parse(v, buf.c_str(), buf.c_str() + strlen(buf.c_str()), &err);
		picojson::object& o = v.get<picojson::object>();
		if (err.empty())
		{
			std::string t = o["access_token"].get<std::string>();
			token = t;
		}
	}

	return buf;
}

// Blocks
std::string Mastodon::GetBlocks(const int & max_id, const int & since_id, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Blocks_String;
	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 80)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			BlockListNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			BlockListPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetBlocksNext()
{
	return BlockListNext;
}

std::string Mastodon::GetBlocksPrev()
{
	return BlockListPrev;
}

// Favourites
std::string Mastodon::GetFavourites(const int & max_id, const int & since_id, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Favourites_String;
	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 80)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			FavouritesNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			FavouritesPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetFavouritesNext()
{
	return FavouritesNext;
}

std::string Mastodon::GetFavouritesPrev()
{
	return FavouritesPrev;
}

// Follow Request
std::string Mastodon::GetFollowRequest(const int& max_id, const int& since_id, const int& limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::FollowReq_String;
	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 80)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			FollowRequestNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			FollowRequestPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetFollowRequestNext()
{
	return FollowRequestNext;
}

std::string Mastodon::GetFollowRequestPrev()
{
	return FollowRequestPrev;
}

bool Mastodon::AuthorizeFollowRequest(const int& id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::AuthorizeFollow_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		return true;
	}

	return false;
}

bool Mastodon::RejectFollowRequest(const int& id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::AccountAPI_String + std::to_string(id) + MSTDNAPI_Strings::RejectFollow_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		return true;
	}

	return false;
}


// Follow
std::string Mastodon::Follow(const std::string& uri)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::UpdatingCurrentUser_String;

	curl_formadd(&post, &last, CURLFORM_COPYNAME, "url", CURLFORM_COPYCONTENTS, url.c_str(), CURLFORM_END);

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPPOST, post);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);
	curl_formfree(post);

	return buf;
}


// instance
std::string Mastodon::GetCurrentInstance()
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Instances_String;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

// Media API
std::string Mastodon::UploadMedia(const std::string& filepath) {
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Media_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string result;
	std::string type = "";
	std::regex mp4("[\\s\\w]*.mp4");
	std::regex webm("[\\s\\w]*.webm");

	if (std::regex_match(filepath, mp4))
	{
		type = "video/mp4";
	}
	else if (std::regex_match(filepath, webm))
	{
		type = "video/webm";
	}
	else
	{
		std::ifstream fs(filepath);
		std::string test = CheckMediaType(fs);
		if (test == "image/jpeg" || test == "image/png" || test == "image/gif")
		{
			type = test;
		}
	}

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &result);

	curl_formadd(&post, &last, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filepath.c_str(), CURLFORM_CONTENTTYPE, type.c_str(), CURLFORM_END);
	curl_easy_setopt(cHandler, CURLOPT_HTTPPOST, post);
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	if (res == CURLM_OK)
	{
		curl_formfree(post);
		curl_slist_free_all(headerList);
		return result;
	}

	curl_formfree(post);
	curl_slist_free_all(headerList);

	return result;
}

// Mute
std::string Mastodon::GetMutes(const int & max_id, const int & since_id, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::FollowReq_String;
	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 80)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			MutesNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			MutesPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetMutesNext()
{
	return MutesNext;
}

std::string Mastodon::GetMutesPrev()
{
	return MutesPrev;
}

std::string Mastodon::GetNotifications(const int& id, const int & max_id, const int & since_id, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = "";

	if (id != 0)
	{
		url += ServerUrl + MSTDNAPI_Strings::Notifications_String + "/" + std::to_string(id);
	}
	else
	{
		url += ServerUrl + MSTDNAPI_Strings::Notifications_String;
	}

	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 30)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			NotificationsNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			NotificationsPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetNotificationsNext()
{
	return NotificationsNext;
}

std::string Mastodon::GetNotificationsPrev()
{
	return NotificationsPrev;
}

bool Mastodon::ClearNotifications()
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::ClearingNotifications_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		return true;
	}

	return false;
}

std::string Mastodon::GetReports()
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Reports_String;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::Reportinguser(const int & account_id, const std::vector<int>& status_ids, const std::string & comment)
{

	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Reports_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;

	int count = 0;
	std::string idsparam = "status_ids[]=";
	std::string accountid_param = "account_id=";

	std::string idstmp = "";
	std::string accountid_tmp = "";

	std::string post_data = "";

	for (int n : status_ids)
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "status_ids[]", CURLFORM_COPYCONTENTS, std::to_string(n).c_str(), CURLFORM_END);
	}

	if(account_id != 0)
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "account_id", CURLFORM_COPYCONTENTS, std::to_string(account_id).c_str(), CURLFORM_END);
	}

	if(comment != "")
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "comment", CURLFORM_COPYCONTENTS, comment.c_str(), CURLFORM_END);
	}

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_HTTPPOST, post);
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);
	curl_formfree(post);

	return buf;
}

std::string Mastodon::Search(const std::string & q, const bool & resolve)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string params = "?q=" + q;

	if (resolve)
	{
		params += "resolve=true";
	}
	else
	{
		params += "resolve=false";
	}

	std::string url = ServerUrl + MSTDNAPI_Strings::Search_String + params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

// Statuses
std::string Mastodon::GetStatus(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id);

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetStatusContext(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id) + "/" + MSTDNAPI_Strings::GetStatusContest_String;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetStatusCard(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id) + MSTDNAPI_Strings::GetCard_String;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetWhoRebloggedStatus(const int & id, const int & max_id, const int & since_id, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url =  ServerUrl + " / " + std::to_string(id) + MSTDNAPI_Strings::GetReblogged_String;

	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 30)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			RebloggedNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			RebloggedPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetRebloggedNext()
{
	return RebloggedNext;
}

std::string Mastodon::GetRebloggedPrev()
{
	return RebloggedPrev;
}

std::string Mastodon::GetWhoFavouritedStatus(const int & id, const int & max_id, const int & since_id, const int & limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + " / " + std::to_string(id) + MSTDNAPI_Strings::GetFavourited_String;

	std::string params = "";

	if (max_id != 0)
	{
		std::string max_id_str = std::to_string(max_id);
		if (params == "")
		{
			params += "max_id=" + max_id_str;
		}
		else
		{
			params += "&max_id=" + max_id_str;
		}
	}

	if (since_id != 0)
	{
		std::string since_id_str = std::to_string(since_id);
		if (params == "")
		{
			params += "since_id=" + since_id_str;
		}
		else
		{
			params += "&since_id=" + since_id_str;
		}
	}

	if (limit != 0 && limit <= 30)
	{
		std::string limit_str = std::to_string(limit);
		if (params == "")
		{
			params += "limit=" + limit_str;
		}
		else
		{
			params += "&limit=" + limit_str;
		}
	}

	url += params;

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//Curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		std::smatch m;

		if (std::regex_search(writeheader, m, next) == 1)
		{
			FavouritedNext = m[1].str();
		}

		if (std::regex_search(writeheader, m, prev) == 1)
		{
			FavouritedPrev = m[1].str();
		}
	}

	return buf;
}

std::string Mastodon::GetFavouritedNext()
{
	return FavouritedNext;
}

std::string Mastodon::GetFavouritedPrev()
{
	return FavouritedPrev;
}

std::string Mastodon::Toot(const std::string& status, const int& in_reply_to_id, std::vector<int> media_ids, bool sensitive, const std::string& spoiler_text, const std::string& visibility)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;

	curl_formadd(&post, &last, CURLFORM_COPYNAME, "status", CURLFORM_COPYCONTENTS, status.c_str(), CURLFORM_END);

	if (sensitive)
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "sensitive", CURLFORM_COPYCONTENTS, "true", CURLFORM_END);
	}

	if (media_ids[0] != 0)
	{
		for (int n : media_ids)
		{
			curl_formadd(&post, &last, CURLFORM_COPYNAME, "media_ids[]", CURLFORM_COPYCONTENTS, std::to_string(n).c_str(), CURLFORM_END);
		}

	}

	if (visibility == "direct" || visibility == "private" || visibility == "unlisted" || visibility == "public")
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "visibility", CURLFORM_COPYCONTENTS, visibility.c_str(), CURLFORM_END);
	}

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_HTTPPOST, post);
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);
	curl_formfree(post);

	return buf;
}

bool Mastodon::DeleteToot(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id);
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	if (res == 0)
	{
		return true;
	}

	return false;
}

std::string Mastodon::Reblog(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id) + MSTDNAPI_Strings::RebloggingStatus_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::UnReblog(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id) + MSTDNAPI_Strings::UnRebloggingStatus_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::Favourite(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id) + MSTDNAPI_Strings::FavouritingStatus_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::UnFavourite(const int & id)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl + MSTDNAPI_Strings::Status_String + "/" + std::to_string(id) + MSTDNAPI_Strings::UnFavouritingStatus_String;
	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;
	std::string buf;
	std::string post_data = "";

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_POST, 1);
	curl_easy_setopt(cHandler, CURLOPT_POSTFIELDSIZE, post_data.length());
	curl_easy_setopt(cHandler, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);

	//curl Start
	res = curl_easy_perform(cHandler);

	curl_slist_free_all(headerList);

	return buf;
}

// TImeLines
std::string Mastodon::GetTimeLines(const std::string& type, const std::string& hashtag, const std::string& local, const int& max_id, const int& since_id, const int& limit)
{
	cHandler = curl_easy_init();
	CURLcode res;

	struct curl_slist *headerList = NULL;

	std::string url = ServerUrl;
	std::string params = "";

	if (type == "home")
	{
		url += MSTDNAPI_Strings::TimeLineHome_String;
	}
	else if (type == "public")
	{
		url += MSTDNAPI_Strings::TimeLinePublic_String;
	}
	else if (type == "hashtag")
	{
		url += MSTDNAPI_Strings::TimeLineHashTag_String;
	}

	if (type == "public" || type == "hashtag")
	{
		if (local != "")
		{
			if(params == "")
			{
				params += "local=" + local;
			}
			else
			{
				params += "&local=" + local;
			}
		}
	}

	if (max_id != 0)
	{
		std::string max_idstr = std::to_string(max_id);
		if(params == "")
		{
			params += "max_id=" + max_idstr;
		}
		else
		{
			params += "&max_id=" + max_idstr;
		}
	}

	if (since_id != 0)
	{
		std::string since_idstr = std::to_string(since_id);
		if(params == "")
		{
			params += "since_id=" + since_idstr;
		}
		else
		{
			params += "&since_id=" + since_idstr;
		}
	}

	if (limit != 0 && limit <= 40)
	{
		std::string limitstr = std::to_string(limit);
		if(params == "")
		{
			params += "limit=" + limitstr;
		}
		else
		{
			params += "&limit=" + limitstr;
		}
	}

	if (params != "")
	{
		url += "?" + params;
	}

	std::string header = MSTDNAPI_Strings::AuthorizeHeader + token;

	std::string buf;
	std::string writeheader;

	headerList = curl_slist_append(headerList, header.c_str());

	curl_easy_setopt(cHandler, CURLOPT_URL, url.c_str());
	curl_easy_setopt(cHandler, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(cHandler, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cHandler, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_WRITEDATA, &buf);
	curl_easy_setopt(cHandler, CURLOPT_HEADERFUNCTION, CurlWriteCallback);
	curl_easy_setopt(cHandler, CURLOPT_HEADERDATA, &writeheader);

	//curl Curl Start
	res = curl_easy_perform(cHandler);

	if (res == 0)
	{
		std::smatch m;
		if (std::regex_search(writeheader, m, next) == 1)
		{
			TLnext = m[1].str();
		}
		if (std::regex_search(writeheader, m, prev) == 1)
		{
			TLprev = m[1].str();
		}
	}

	curl_slist_free_all(headerList);

	return buf;
}

std::string Mastodon::GetTimeLineNext()
{
	return TLnext;
}

std::string Mastodon::GetTimeLinePrev()
{
	return TLprev;
}
