#ifndef MASTODON_H
#define MASTODON_H

#include <curl/curl.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <sstream>
#include "base64.h"
#include "picojson/picojson.h"
#include "MSTDN_APIList.h"

class Mastodon
{

private:
	std::string InstanceName;
	std::string ServerUrl;
	std::string token;

	std::string TLnext;
	std::string TLprev;
	std::string FollowingNext;
	std::string FollowingPrev;
	std::string FollowersNext;
	std::string FollowersPrev;
	std::string AccountsStatusNext;
	std::string AccountsStatusPrev;
	std::string BlockListNext;
	std::string BlockListPrev;
	std::string FavouritesNext;
	std::string FavouritesPrev;
	std::string FollowRequestNext;
	std::string FollowRequestPrev;
	std::string MutesNext;
	std::string MutesPrev;
	std::string NotificationsNext;
	std::string NotificationsPrev;
	std::string RebloggedNext;
	std::string RebloggedPrev;
	std::string FavouritedNext;
	std::string FavouritedPrev;

	std::regex next;
	std::regex prev;

	CURL *cHandler;

	std::string readbuffer;

	std::vector<std::string> responseJSON;

public:
	Mastodon(const std::string& HostName, const std::string& TokenStr = "");
	virtual ~Mastodon();

	// Tools
	static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

	std::string ImageToBase64(std::ifstream& fs);
	std::string CheckMediaType(std::ifstream& fs);

	std::string Base64Safe(std::string & str);

	void SetInstanceUrl(const std::string& instancename);

	// Accounts
	std::string GetAccount(const int& id = 0);
	std::string UpdateAccount(const std::string display_name = "", const std::string note = "", const std::string avatar = "", const std::string header = "");
	std::string GetFollowers(const int& id, const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetFollowersNext();
	std::string GetFollowersPrev();
	std::string GetFollowing(const int& id, const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetFollowingNext();
	std::string GetFollowingPrev();
	std::string GetAccountsStatuses(const int& id, bool only_media = false, bool exclude_replies = false, const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetAccountsStatusNext();
	std::string GetAccountsStatusPrev();
	std::string GetAccountsFollow(const int& id);
	std::string GetAccountsUnFollow(const int& id);
	std::string GetAccountsBlock(const int& id);
	std::string GetAccountsUnBlock(const int& id);
	std::string GetAccountsMute(const int& id);
	std::string GetAccountsUnMute(const int& id);
	std::string GetAccountsRelationships(const std::vector<int>& ids);
	std::string AccountsSearch(const std::string& q, const int& limit = 40);

	// Apps
	std::string CreateApp(const std::string& AppName, const std::vector<std::string>& scopes, const std::string& redirect = "", const std::string& website = "", const std::string& fname = "");

	// Get Token
	std::string GetToken(const std::string &client_id, const std::string &id_secret, const std::string &Mail, const std::string &Pass, const std::string& fname = "");

	// Blocks
	std::string GetBlocks(const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetBlocksNext();
	std::string GetBlocksPrev();

	// Favourites
	std::string GetFavourites(const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetFavouritesNext();
	std::string GetFavouritesPrev();

	// Follow Request
	std::string GetFollowRequest(const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetFollowRequestNext();
	std::string GetFollowRequestPrev();
	bool AuthorizeFollowRequest(const int& id);
	bool RejectFollowRequest(const int& id);

	// Follows
	std::string Follow(const std::string& uri);

	// Instance
	std::string GetCurrentInstance();

	// Media
	std::string UploadMedia(const std::string& filepath);

	// Mute
	std::string GetMutes(const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetMutesNext();
	std::string GetMutesPrev();

	// Notifications
	std::string GetNotifications(const int& id = 0, const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetNotificationsNext();
	std::string GetNotificationsPrev();
	bool ClearNotifications();

	// Reports
	std::string GetReports();
	std::string Reportinguser(const int& account_id, const std::vector<int>& status_ids, const std::string& comment);

	// Search
	std::string Search(const std::string& q, const bool& resolve);
	
	// Statuses
	std::string GetStatus(const int& id);
	std::string GetStatusContext(const int& id);
	std::string GetStatusCard(const int& id);
	std::string GetWhoRebloggedStatus(const int& id, const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetRebloggedNext();
	std::string GetRebloggedPrev();
	std::string GetWhoFavouritedStatus(const int& id, const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetFavouritedNext();
	std::string GetFavouritedPrev();
	std::string Toot(const std::string& Text, const int& in_reply_to_id = 0, std::vector<int> media_ids = { 0 }, bool sensitive = false, const std::string& spoiler_text = "", const std::string& visibility = "");
	bool DeleteToot(const int& id);
	std::string Reblog(const int& id);
	std::string UnReblog(const int& id);
	std::string Favourite(const int& id);
	std::string UnFavourite(const int& id);

	// Timelines
	std::string GetTimeLines(const std::string& type, const std::string& hashtag = "", const std::string& local = "", const int& max_id = 0, const int& since_id = 0, const int& limit = 0);
	std::string GetTimeLineNext();
	std::string GetTimeLinePrev();

};

#endif // !MASTODON_H

