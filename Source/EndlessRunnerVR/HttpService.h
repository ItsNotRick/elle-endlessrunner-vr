//
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Http.h"
#include "Base64.h"
#include "StringConv.h"
#include "HttpService.generated.h"

UENUM(BlueprintType, Category = "HTTPServ")
enum class RequestResult : uint8
{
	Success,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHSOnResult, const RequestResult, responseResult);

USTRUCT(BlueprintType)
struct FRequest_Register {
	GENERATED_BODY()
		UPROPERTY(BlueprintReadWrite) FString username;
	UPROPERTY(BlueprintReadWrite) FString password;
	//UPROPERTY() FString sex;
	//UPROPERTY() int age;
	//UPROPERTY() FString motivation;

	FRequest_Register() {}
};

USTRUCT()
struct FResponse_Register {
	GENERATED_BODY()
		UPROPERTY() bool success;

	FResponse_Register() {}
};

USTRUCT(BlueprintType)
struct FRequest_Login {
	GENERATED_BODY()
		UPROPERTY(BlueprintReadWrite) FString username;
	UPROPERTY(BlueprintReadWrite) FString password;

	FRequest_Login() {}
};

USTRUCT()
struct FResponse_Login {
	GENERATED_BODY()
		UPROPERTY() FString access_token;
	UPROPERTY() FString permissions;
	UPROPERTY() int id;

	FResponse_Login() {}
};

USTRUCT(BlueprintType)
struct FResponse_Decks {
	GENERATED_BODY()
		UPROPERTY(BlueprintReadOnly) TArray<int> ids;
	UPROPERTY(BlueprintReadOnly) TArray<FString> names;
	FResponse_Decks() {}
};

USTRUCT()
struct FRequest_Leaderboard {
	GENERATED_BODY()
		UPROPERTY() int deckID;

	FRequest_Leaderboard() {}
};

USTRUCT(BlueprintType)
struct FResponse_Leaderboard {
	GENERATED_BODY()
		UPROPERTY(BlueprintReadOnly) TArray<FString> username;
	UPROPERTY(BlueprintReadOnly) TArray<int> scores;
	UPROPERTY(BlueprintReadOnly) TArray<FString> timeElapsed;

	FResponse_Leaderboard() {}
};

UCLASS(Blueprintable, Category = "HTTPServ")
class ENDLESSRUNNERVR_API UHttpService : public UObject
{
	GENERATED_BODY()

private:
	FHttpModule* Http;
	FString ApiBaseUrl = "http://endlesslearner.com/";
	FResponse_Register RegisterResponseData;
	FResponse_Login LoginResponseData;
	FResponse_Decks DecksResponseData;
	FResponse_Leaderboard LeaderboardResponseData;


	FString AuthorizationHeader = TEXT("Authorization");
	void SetAuthorizationHash(FString Hash, TSharedRef<IHttpRequest>& Request);

	TSharedRef<IHttpRequest> RequestWithRoute(FString Subroute);
	void SetRequestHeaders(TSharedRef<IHttpRequest>& Request);

	TSharedRef<IHttpRequest> GetRequest(FString Subroute);
	TSharedRef<IHttpRequest> GetRequestTwo(FString Subroute, FString Token);
	TSharedRef<IHttpRequest> PostRequest(FString Subroute, FString ContentJsonString);
	void Send(TSharedRef<IHttpRequest>& Request);

	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);

	template <typename StructType>
	void GetJsonStringFromStruct(StructType FilledStruct, FString& StringOutput);
	template <typename StructType>
	void GetStructFromJsonString(FHttpResponsePtr Response, StructType& StructOutput);

public:
	UHttpService();

	UPROPERTY(BlueprintReadOnly)
		FString access_token;

	UPROPERTY(BlueprintReadOnly)
		int userID;

	UPROPERTY(BlueprintReadOnly)
		FResponse_Decks deck_response;

	UPROPERTY(BlueprintReadOnly)
		FResponse_Leaderboard leaderboard_response;

	UPROPERTY(BlueprintAssignable, Category = "HTTPServ")
		FHSOnResult OnResponseResult;

	UFUNCTION(BlueprintCallable, Meta = (DisplayName = "Create Request"), Category = "HTTPServ")
		static UHttpService* CreateNewConnection();

	UFUNCTION(BlueprintCallable, Category = "HTTPServ")
		UHttpService* Register(FRequest_Register RegisterCredentials);

	void RegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "HTTPServ")
		UHttpService* Login(FRequest_Login LoginCredentials);
	
	void LoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "HTTPServ")
		UHttpService* getDecks(FString token);

	void decksResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "HTTPServ")
		UHttpService* createLeaderboard(int deckID, FString token);
	
	void leaderboardResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable)
		void postCSV(FString fileName, FString fileLocation, FString token);
};