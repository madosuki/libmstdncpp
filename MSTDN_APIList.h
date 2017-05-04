#ifndef MSTDN_APILIST_H
#define MSTDN_APILIST_H

#include <string>

namespace MSTDNAPI_Strings
{
	// API BASE URL
	const std::string apiBase = "/api/v1/";

	// Default ID
	const std::string ID_String = ":id";

	// Authorize Header
	const std::string AuthorizeHeader = "Authorization: Bearer ";

	// Accounts
	const std::string AccountAPI_String = apiBase + "accounts/";
	const std::string GetAccount_String = AccountAPI_String;
	const std::string GetCurrentUser_String = AccountAPI_String + "verify_credentials";
	const std::string UpdatingCurrentUser_String = AccountAPI_String + "update_credentials";
	const std::string GettingAccountsFollowers_String = "/followers";
	const std::string GettingAccountsFollowing_String = "/following";
	const std::string GettingAccountsStatuses_String = "/statuses";
	const std::string FollowAccount_String = "/follow";
	const std::string UnFollowingAccount_String = "/unfollow";
	const std::string AccountBlocking_String = "/block";
	const std::string AccountUnBlocking_String =  "/unblock";
	const std::string AccountMuting_String = "/mute";
	const std::string AccountUnMuting_String = "/unmute";
	const std::string AccountRelationShips = "relationships";
	const std::string AccountSearch_String = "search";

	// Apps
	const std::string CreateAppAPI_String = apiBase + "apps";

	// Blocks Fetching a user's blocks
	const std::string Blocks_String = apiBase + "blocks";

	// Favourites
	const std::string Favourites_String = apiBase + "favourites";

	// Follow Requests
	const std::string FollowReq_String = apiBase + "follow_requests";
	const std::string AuthorizeFollow_String = "/authorize";
	const std::string RejectFollow_String = "/reject";

	// Follows
	const std::string Follow_String = apiBase + "follows";

	// Instances
	const std::string Instances_String = apiBase + "instance";

	// Media
	const std::string Media_String = apiBase + "media";

	// Mutes
	const std::string Mutes_Strins = apiBase + "mutes";

	// Notifications
	const std::string Notifications_String = apiBase + "notifications";
	const std::string ClearingNotifications_String = Notifications_String + "/" + "clear";

	// Reports
	const std::string Reports_String = apiBase + "reports";

	// Search : Searching for content
	const std::string Search_String = apiBase + "search";

	// Statueses
	const std::string Status_String = apiBase + "statuses";
	const std::string GetStatusContest_String = "/context";
	const std::string GetCard_String = "/card";
	const std::string GetReblogged_String = "/reblogged_by";
	const std::string GetFavourited_String = "/favourited_by";
	const std::string RebloggingStatus_String = "/reblog";
	const std::string UnRebloggingStatus_String = "/unreblog";
	const std::string FavouritingStatus_String = "/favourite";
	const std::string UnFavouritingStatus_String = "/unfavourite";

	// TimeLines
	const std::string TimeLine_String = apiBase + "timelines/";
	const std::string TimeLineHome_String = TimeLine_String + "home";
	const std::string TimeLinePublic_String = TimeLine_String + "public";
	const std::string TimeLineHashTag_String = TimeLine_String + "tag/";
}

#endif // !MSTDN_APILIST_H
