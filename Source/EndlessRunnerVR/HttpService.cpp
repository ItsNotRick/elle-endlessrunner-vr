// 

#include "HttpService.h"
#include "EndlessRunnerVR.h"
#include <iostream>
#include <string>

UHttpService::UHttpService()
{
}


UHttpService* UHttpService::CreateNewConnection(void)
{
	UE_LOG(LogTemp, Warning, TEXT("Connecting to API"));
	UHttpService* http_object = NewObject<UHttpService>();
	return http_object;
}

/**************************************************************************************************************************/

TSharedRef<IHttpRequest> UHttpService::RequestWithRoute(FString Subroute) {
	Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(ApiBaseUrl + Subroute);
	SetRequestHeaders(Request);
	return Request;
}

void UHttpService::SetRequestHeaders(TSharedRef<IHttpRequest>& Request) {
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
}

TSharedRef<IHttpRequest> UHttpService::GetRequest(FString Subroute) {
	TSharedRef<IHttpRequest> Request = RequestWithRoute(Subroute);
	Request->SetVerb("GET");
	return Request;
}

TSharedRef<IHttpRequest> UHttpService::GetRequestTwo(FString Subroute, FString token) {
	TSharedRef<IHttpRequest> Request = RequestWithRoute(Subroute);
	Request->SetHeader(TEXT("Authorization"), token);
	Request->SetVerb("GET");
	return Request;
}

TSharedRef<IHttpRequest> UHttpService::PostRequest(FString Subroute, FString ContentJsonString) {
	TSharedRef<IHttpRequest> Request = RequestWithRoute(Subroute);
	Request->SetVerb("POST");
	Request->SetContentAsString(ContentJsonString);
	return Request;
}

void UHttpService::Send(TSharedRef<IHttpRequest>& Request) {
	Request->ProcessRequest();
	AddToRoot();
}

bool UHttpService::ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful) {
	if (!bWasSuccessful || !Response.IsValid()) return false;
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(Response->GetContentAsString()));
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode())) return true;
	else {
		return false;
	}
}

void UHttpService::SetAuthorizationHash(FString Hash, TSharedRef<IHttpRequest>& Request) {
 	Request->SetHeader(AuthorizationHeader, Hash);
}



/**************************************************************************************************************************/



template <typename StructType>
void UHttpService::GetJsonStringFromStruct(StructType FilledStruct, FString& StringOutput) {
	FJsonObjectConverter::UStructToJsonObjectString(StructType::StaticStruct(), &FilledStruct, StringOutput, 0, 0);
}

template <typename StructType>
void UHttpService::GetStructFromJsonString(FHttpResponsePtr Response, StructType& StructOutput) {
	StructType StructData;
	FString JsonString = Response->GetContentAsString();
	FJsonObjectConverter::JsonObjectStringToUStruct<StructType>(JsonString, &StructOutput, 0, 0);
}



/**************************************************************************************************************************/

void UHttpService::postCSV(FString fileName, FString fileLocation, FString token)
{
	UE_LOG(LogTemp, Warning, TEXT("4.PostCSV Begin"));
	UE_LOG(LogTemp, Warning, TEXT("Getting PostSV for access_token: %s"), *token);
	// Get Data
	TArray<uint8> UpFileRawData;
	FString FilePath = fileLocation + fileName;
	UE_LOG(LogTemp, Warning, TEXT("PostCSV Complete File Path: %s"), *FilePath);
	FFileHelper::LoadFileToArray(UpFileRawData, *FilePath);

	// prepare json data
	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);

	JsonWriter->WriteObjectStart();
	  	//TJsonWriter< TCHAR, PrintPolicy >
	JsonWriter->WriteValue(fileName, FilePath);
	JsonWriter->WriteValue("CSV", UpFileRawData);
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();

	FString path = "session";
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	//Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	//Request->SetHeader("Content-Disposition", "form-data; name=fileToUpload; filename=" + fileName);
	//Request->SetHeader(TEXT("Content-Type"), TEXT("multipart/form-data"));
	//Request->SetHeader(TEXT("Content-Type"), TEXT("text/csv"));
	//Request->SetHeader(TEXT("Authorization"), token);
	//Request->SetHeader(TEXT("Content-Length"), FString::FromInt(UpFileRawData.Num()));
	//Request->SetHeader(TEXT("Content-Disposition"), "form-data; filename="+fileName);
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("text/csv"));
	Request->SetHeader(TEXT("Authorization"), access_token);
	Request->SetVerb("POST");
	UE_LOG(LogTemp, Warning, TEXT("CSV UPLOAD URL for: %s"), *path);
	Request->SetURL(ApiBaseUrl + path);
	Request->SetContentAsString(JsonString);
	//Request->SetContent(UpFileRawData);
	Request->ProcessRequest();
//	AddToRoot();
	UE_LOG(LogTemp, Warning, TEXT("PostCSV End"));
}


//////////////////////////////////////// REGISTER FUNCTIONS BEGIN ////////////////////////////////////////
UHttpService* UHttpService::Register(FRequest_Register RegisterCredentials) {
	FString ContentJsonString;
	GetJsonStringFromStruct<FRequest_Register>(RegisterCredentials, ContentJsonString);

	TSharedRef<IHttpRequest> Request = PostRequest("register", ContentJsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpService::RegisterResponse);
	Send(Request);

	return this;
}

void UHttpService::RegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	RemoveFromRoot();
	Request->OnProcessRequestComplete().Unbind();
	if (!ResponseIsValid(Response, bWasSuccessful)) {
		RegisterResponseData.success = false;
		OnResponseResult.Broadcast(RequestResult::Failed);
		return;
	}
	else {
		GetStructFromJsonString<FResponse_Register>(Response, RegisterResponseData);
		OnResponseResult.Broadcast(RequestResult::Success);
	}
	//GetStructFromJsonString<FResponse_Register>(Response, RegisterResponseData);
}
//////////////////////////////////////// REGISTER FUNCTIONS END ////////////////////////////////////////


//////////////////////////////////////// LOGIN FUNCTIONS BEGIN ////////////////////////////////////////
UHttpService* UHttpService::Login(FRequest_Login LoginCredentials) {
	FString ContentJsonString;
	UE_LOG(LogTemp, Warning, TEXT("Login Credentials: Username: %s Password: %s"), *LoginCredentials.username, *LoginCredentials.password);
	GetJsonStringFromStruct<FRequest_Login>(LoginCredentials, ContentJsonString);

	TSharedRef<IHttpRequest> Request = PostRequest("login", ContentJsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpService::LoginResponse);
	Send(Request);

	return this;
}

void UHttpService::LoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	RemoveFromRoot();
	Request->OnProcessRequestComplete().Unbind();
	if (!ResponseIsValid(Response, bWasSuccessful)) {
		OnResponseResult.Broadcast(RequestResult::Failed);
		return;
	}else{
		GetStructFromJsonString<FResponse_Login>(Response, LoginResponseData);
		access_token = "Bearer " + LoginResponseData.access_token;
		userID = LoginResponseData.id;
		OnResponseResult.Broadcast(RequestResult::Success);
	}
}
//////////////////////////////////////// LOGIN FUNCTIONS END ////////////////////////////////////////


//////////////////////////////////////// DECK FUNCTIONS BEGIN ///////////////////////////////////////
UHttpService* UHttpService::getDecks(FString token) {
	UE_LOG(LogTemp, Warning, TEXT("Getting decks for access_token: %s"), *token);
	FString path = "decks";
	TSharedRef<IHttpRequest> Request = GetRequestTwo(path, token);
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpService::decksResponse);
	Send(Request);

	return this;
}

void UHttpService::decksResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	RemoveFromRoot();
	Request->OnProcessRequestComplete().Unbind(); 
	if (!ResponseIsValid(Response, bWasSuccessful)) {
		OnResponseResult.Broadcast(RequestResult::Failed);
		return;
	}
	else {

		GetStructFromJsonString<FResponse_Decks>(Response, DecksResponseData);
		deck_response = DecksResponseData;
		OnResponseResult.Broadcast(RequestResult::Success);
	}

	//GetStructFromJsonString<FResponse_Decks>(Response, DecksResponseData);
}
//////////////////////////////////////// DECK FUNCTIONS END ///////////////////////////////////////


//////////////////////////////////////// LEADERBOARD FUNCTIONS BEGIN ///////////////////////////////////////
UHttpService* UHttpService::createLeaderboard(int deckID, FString token) {
	UE_LOG(LogTemp, Warning, TEXT("Creating Leaderboard for Deck: %d"), deckID);
	FString path = "leaderboard/" + FString::FromInt(deckID);
	UE_LOG(LogTemp, Warning, TEXT("Passing Leaderboard Access Token %s:"), *token);
	TSharedRef<IHttpRequest> Request = GetRequestTwo(path, token);
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpService::leaderboardResponse);
	Send(Request);

	return this;
}


void UHttpService::leaderboardResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	RemoveFromRoot();
	Request->OnProcessRequestComplete().Unbind();
	if (!ResponseIsValid(Response, bWasSuccessful)) {
		OnResponseResult.Broadcast(RequestResult::Failed);
		return;
	}
	else {

		GetStructFromJsonString<FResponse_Leaderboard>(Response, LeaderboardResponseData);
		leaderboard_response = LeaderboardResponseData;
		OnResponseResult.Broadcast(RequestResult::Success);
	}


}
	// ----------------------- SAMPLE DATA -----------------------//
	//LeaderboardResponseData.scores = { 75, 40, 32, 20 ,19};
	//LeaderboardResponseData.username = { "Admin", "userAlpha", "userBeta","userCharlie","userDelta" };
	//LeaderboardResponseData.timeElapsed = { "00:10:00", "00:07:47", "00:05:52", "00:04:41", "00:04:25"};
	// ----------------------- SAMPLE DATA -----------------------//

	// SAMPLE QUERY FOR LEADERBOARD
	/* SELECT user.username, session.playerScore, session.elapsedTime
		FROM session INNER JOIN
			round ON session.sessionID = round.sessionID INNER JOIN
			user ON user.userID = session.userID INNER JOIN
			loggedanswers ON loggedanswers.roundID = round.roundID INNER JOIN
			card ON loggedanswers.cardID = card.cardID
				WHERE card.deckID = 300
        GROUP BY session.sessionID ORDER BY session.playerScore DESC;
	*/
//////////////////////////////////////// LEADERBOARD FUNCTIONS END ///////////////////////////////////////