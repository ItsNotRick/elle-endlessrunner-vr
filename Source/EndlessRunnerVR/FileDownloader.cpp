// Fill out your copyright notice in the Description page of Project Settings.

#include "FileDownloader.h"
#include "EndlessRunnerVR.h"
#include "GenericPlatformFile.h"
#include "PlatformFilemanager.h"


UFileDownloader::UFileDownloader():
	FileUrl(TEXT(""))
	, FileSavePath(TEXT(""))
{
}

UFileDownloader::~UFileDownloader()
{
}

void UFileDownloader::SetRequestHeaders(TSharedRef<IHttpRequest>& Request) {
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
}

TSharedRef<IHttpRequest> UFileDownloader::RequestWithRoute(FString Subroute) {
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(ApiBaseUrl + Subroute);
	SetRequestHeaders(Request);
	return Request;
}

TSharedRef<IHttpRequest> UFileDownloader::GetRequestTwo(FString Subroute, FString token) {
	TSharedRef<IHttpRequest> Request = RequestWithRoute(Subroute);
	Request->SetHeader(TEXT("Authorization"), token);
	Request->SetVerb("GET");
	return Request;
}

UFileDownloader* UFileDownloader::MakeDownloader()
{
	UFileDownloader* Downloader = NewObject<UFileDownloader>();
	return Downloader;
}

UFileDownloader* UFileDownloader::DownloadFile(FString SavePath,FString FileName, FString access_token, int deckID)
{
	FString deckIDSTR = FString::FromInt(deckID);
	FileUrl = "deck/zip/" + deckID;
	FileSavePath = SavePath + FileName;

	//UE_LOG(LogTemp, Warning, TEXT("File URL for DownloadFile: %s"), *FileUrl);
	UE_LOG(LogTemp, Warning, TEXT("Access Token: %s"), *access_token);
	UE_LOG(LogTemp, Warning, TEXT("Local path for file download: %s"), *FileSavePath);
	//TSharedRef< IHttpRequest > HttpRequest = GetRequestTwo("deck/zip/" + deckID, access_token);
	TSharedRef< IHttpRequest > HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	//HttpRequest->SetHeader(TEXT("Accepts"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), access_token);
	HttpRequest->SetVerb("GET");
	FString path = ApiBaseUrl + "deck/zip/" + deckIDSTR;
	UE_LOG(LogTemp, Warning, TEXT("File URL for DownloadFile: %s"), *path);
	HttpRequest->SetURL(path);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UFileDownloader::OnReady);
	HttpRequest->OnRequestProgress().BindUObject(this, &UFileDownloader::OnProgress_Internal);

	// Execute the request
	HttpRequest->ProcessRequest();
	AddToRoot();

	return this;
}

void UFileDownloader::OnReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	RemoveFromRoot();
	Request->OnProcessRequestComplete().Unbind();

	if (Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		// SAVE FILE
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// create save directory if not existent
		FString Path, Filename, Extension;
		FPaths::Split(FileSavePath, Path, Filename, Extension);
		if (!PlatformFile.DirectoryExists(*Path))
		{
			if(!PlatformFile.CreateDirectoryTree(*Path))
			{
				OnResult.Broadcast(EDownloadResult::DirectoryCreationFailed);
				return;
			}
		}

		// open/create the file
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*FileSavePath);
		if (FileHandle)
		{
			// write the file
			FileHandle->Write(Response->GetContent().GetData(), Response->GetContentLength());
			// Close the file
			delete FileHandle;

			OnResult.Broadcast(EDownloadResult::Success);
		}
		else
		{
			OnResult.Broadcast(EDownloadResult::SaveFailed);
		}
	}
	else
	{
		OnResult.Broadcast(EDownloadResult::DownloadFailed);
	}
}

void UFileDownloader::OnProgress_Internal(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
{
	int32 FullSize = Request->GetContentLength();
	OnProgress.Broadcast(BytesSent, BytesReceived, FullSize);
}