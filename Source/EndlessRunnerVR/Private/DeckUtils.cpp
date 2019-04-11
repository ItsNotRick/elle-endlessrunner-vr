// Fill out your copyright notice in the Description page of Project Settings.

#include "DeckUtils.h"
#include "rapidcsv.h"
#include <vector>
#include <string>


TArray<FCard> UDeckUtils::extractFromCSV(FString path) {
	TArray<FCard> ret;
	rapidcsv::Document doc(std::string(TCHAR_TO_UTF8(*path)));

	auto ev = doc.GetColumn<std::string>("English");
	auto tv = doc.GetColumn<std::string>("Translation");
	ret.Reserve(ev.size() + 2);
	for (int i = 0; i < ev.size(); i++) {
		ret.AddDefaulted();
		ret[i] = FCard { FString(UTF8_TO_TCHAR(ev[i].c_str())), FString(UTF8_TO_TCHAR(tv[i].c_str())) };
	}

	return ret;
}


