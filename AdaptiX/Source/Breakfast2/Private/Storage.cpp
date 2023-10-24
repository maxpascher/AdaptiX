// Fill out your copyright notice in the Description page of Project Settings.

#include "Storage.h"
#include <map>
#include <Engine/Engine.h>
#include <fstream>
#include <iostream>
#include <string>
#include <Engine/Texture2D.h>
#include "Containers/UnrealString.h"
#include "Misc/RuntimeErrors.h"

// File Elements
#define INIT_FILE_NAME	"setup.txt"
#define REPLAY_DATA_FOLDER "ReplayData/"

// Init file 
#define COMMENT_SIGN '#'
#define PLAY_REPLAY "PlayReplay"
#define STORAGE_PATH_NAME "StoragePath" 
#define ITERATOR_FILE "IteratorFile"
#define RECORD_POSITION_CSV "Record_Position_CSV" 
#define RECORDING_START_MANUALLY "Recording_Start_Manually"
#define STORE_IMAGES "Store_Images" 
#define IMAGES_HIGH_RESOLUTION "High_Res_Images"
#define STORE_HEADSET_IMAGES "Store_Headset_Images"
#define STORAGE_FREQUENCY "Storage_Frequency"

// Variable storage for the setup file variables
bool Play_Replay = false;						//PLAY_REPLAY
FString Storage_Path;							//STORAGE_PATH_NAME
FString iteratorFile;							//ITERATOR_FILE
bool Recording_Actor_Positions = false;			//RECORD_POSITION_CSV
bool Recording_Start_Manually = false;			//RECORDING_START_MANUALLY
bool Store_Images = false;						//STORE_IMAGES
bool Store_Images_High_Resolution = false;		//IMAGES_HIGH_RESOLUTION
bool Store_Headset_Images = false;				//STORE_HEADSET_IMAGES
float Storage_Frequency;						//STORAGE_FREQUENCY

// InOut Filehandlers and Streams
IFileHandle* csvFileHandle = nullptr; 
std::ifstream csvFileStream; 

/** Prints a string to the unreal screen 
 * @param str String to be print
 * @param log Whether to print to the UE log
 */
void print(FString str, bool log = true) {
	GEngine->AddOnScreenDebugMessage(
		-1,        // don't over wrire previous message, add a new one
		5.0f,   // Duration of message - limits distance messages scroll onto screen
		FColor::Cyan.WithAlpha(255),   // Color and transparancy!
		FString::Printf(TEXT("%s"), *str)  // Our usual text message format
	);
	if (log) {
		UE_LOG(LogTemp, Error, TEXT("%s"), *str);
	}
}

/**
 * Loads the iterator file and the value stored. 
 * If no file is found, creates one and initialises the value to 0.
 */
void loadIteratorFile(FString fileName) {
	FString content;			//Variable to store content of the iterator file
	int iterator_num = 0;		//Iteration number to be read from the content. 0 if file not found.
	// try to load content from file
	iteratorFile = Storage_Path + fileName;
	bool loadSuccessfull = FFileHelper::LoadFileToString(content, *(iteratorFile));
	if(loadSuccessfull){
		content.RemoveFromEnd("\r");
		content.RemoveFromEnd("\n");
		if (content.IsNumeric()) {	// if load sucessfull, save number 
			iterator_num = FCString::Atoi(*content);
		}
	} 
	if(iterator_num == -1) {
		FFileHelper::SaveStringToFile(FString::FromInt(0), *(iteratorFile));
	}
	Storage_Path.AppendInt(iterator_num);
	Storage_Path.Append("/");
}

void UStorage::SetPlayerToSpectatorMode(APlayerController* player) {
	player->ChangeState(NAME_Spectating);
}

bool UStorage::incrementIteratorFile() {
	FString content;
	int iterator_num = 0;
	bool loadSuccessfull = FFileHelper::LoadFileToString(content, *(iteratorFile));
	if (loadSuccessfull){
		content.RemoveFromEnd("\r");
		content.RemoveFromEnd("\n");
		if(content.IsNumeric()) 	// if load sucessfull, save number 
			iterator_num = FCString::Atoi(*content);
	}
	return FFileHelper::SaveStringToFile(FString::FromInt(iterator_num + 1), *(iteratorFile));
}

void UStorage::clearCSVWriterHandle() {
	csvFileHandle = nullptr;
}

/** Sets default values to all stored data */
void setDefault()
{
	Play_Replay = false;
	Storage_Path = "";
	iteratorFile = "";
	Recording_Actor_Positions = false;
	Recording_Start_Manually = false;
	Store_Images = false;
	Store_Images_High_Resolution = false;
	Store_Headset_Images = false;
	Storage_Frequency = 0;
	csvFileHandle = nullptr;
}

bool UStorage::loadInitFile()
{
	setDefault();
	FString theBasePath = FString(FPaths::ProjectDir());
	TArray<FString> setupStrings;
	bool initFileLoaded = FFileHelper::LoadFileToStringArray(setupStrings, *FString(theBasePath + INIT_FILE_NAME));
	if (initFileLoaded) {
		for (FString& f : setupStrings) {
			for (int i = 0; i < f.Len(); i++) {	// Allow Comments, but remove them from 
				if (f[i] == COMMENT_SIGN)
					f = f.Left(i);
			}
			if (f.IsEmpty()) {	// skip empty lines
				continue;
			}
			FString left, right;
			f = f.ConvertTabsToSpaces(1);	// Remove Tabs
			f.RemoveSpacesInline();			// remove Spaces
			f.Split("=", &left, &right);
			if (!left.Compare(PLAY_REPLAY)) {
				Play_Replay = right.ToBool();
			}
			else if (!left.Compare(STORAGE_PATH_NAME)) {
				Storage_Path = right;
				Storage_Path.AppendChar('/');
			}
			else if (!left.Compare(ITERATOR_FILE)) {
				loadIteratorFile(right);
			}
			else if (!left.Compare(RECORD_POSITION_CSV)) {
				Recording_Actor_Positions = right.ToBool();
			}
			else if (!left.Compare(STORE_IMAGES)) {
				Store_Images = right.ToBool();
			}
			else if (!left.Compare(IMAGES_HIGH_RESOLUTION)) {
				Store_Images_High_Resolution = right.ToBool();
			}
			else if (!left.Compare(STORAGE_FREQUENCY)) {
				Storage_Frequency = FCString::Atof(*right);
			}
			else if (!left.Compare(RECORDING_START_MANUALLY)) {
				Recording_Start_Manually = right.ToBool();
			}
			else if (!left.Compare(STORE_HEADSET_IMAGES)) {
				Store_Headset_Images = right.ToBool();
			}

		}
	}
	else {
		print("Error: Could not find init File at" + FString(theBasePath + "setup.txt"), true);
	}

	return initFileLoaded;
}

bool UStorage::FileSaveString(FString Path, const FString FileName, const FString SaveString)
{
	if (Recording_Actor_Positions)
		if (Play_Replay) {
			print("File Saving Deactivated. Replay Running", true);
			return false;
		}
		else {
			return FFileHelper::SaveStringToFile(SaveString, *(Storage_Path + Path + FileName));
		}
	else
		return false;
}


bool UStorage::createReplayDataDirectory(const float GameTime, FString& CurrentPath) {
	CurrentPath = REPLAY_DATA_FOLDER;
	CurrentPath.AppendInt(GameTime * 1000);
	CurrentPath += "/";

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*(Storage_Path + CurrentPath))) {
		PlatformFile.CreateDirectoryTree(*(Storage_Path + CurrentPath));

		if (!PlatformFile.DirectoryExists(*(Storage_Path + CurrentPath))) {
			return false;
		}
	}
	return true;
}

void UStorage::loadFolderNames(TArray<FString>& Folders) {
	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFiles(Folders, *(Storage_Path + "*"), false, true);
}
FString UStorage::GetStoragePath() {
	return Storage_Path;
}
bool UStorage::GetStorage_Activated_Images() {
	return Store_Images;
}
bool UStorage::GetImagesInHighRes() {
	return Store_Images_High_Resolution;
}
bool UStorage::GetStorage_Activated_Headset_Images() {
	return Store_Headset_Images;
}


float UStorage::GetStorageTimerTime() {
	if (Storage_Frequency == 0) {
		UE_LOG(LogTemp, Error, TEXT("Error, storage Frequency is 0"));
		return 0;
	}
	return 1.0 / Storage_Frequency;
}

bool UStorage::GetPlayReplay() {
	return Play_Replay;
}

bool UStorage::GetRecordingActivated() {
	return Recording_Actor_Positions;
}

bool UStorage::GetRecordingStartManually() {
	return Recording_Start_Manually;
}

void UStorage::GetQuat(const FRotator in, FQuat& out) {
	out = FQuat(in);
}

FString UStorage::Conv_QuatToString(FQuat in) {
	const int buffer_length = 15 * 4 + 4 * 4; // length of buffer = ( precision + '0., ' ) *4
	char myStr[buffer_length];
	snprintf(myStr, buffer_length, "%.15f, %.15f, %.15f, %.15f", in.X, in.Y, in.Z, in.W);
	return myStr;
}

void UStorage::LoadFileToString(const FString filename, const FString path, FString& out) {
	FFileHelper::LoadFileToString(out, *(Storage_Path + path + filename));
}


void UStorage::LoadFileToStringArray(const FString filename, const FString path, TArray<FString>& out) {
	FFileHelper::LoadFileToStringArray(out, *(Storage_Path + path + filename));
}

/** Combines the oldPath with an alternative path prefix
 * If the object is local, replace "/Game/" with "/Game/alt_path_prefix"
 */
FString getAlternatePath(FString oldPath, FString alt_path_prefix) {
	TArray<FString> pathData;
	oldPath.ParseIntoArray(pathData, *(FString("/")));
	if (!pathData[0].Compare("Game")) { // If the object is local, replace "/Game/" with "/Game/alt_path_prefix"
		pathData[0] = FPaths::Combine(FString("/")+pathData[0], alt_path_prefix);
	}
	return FString::Join(pathData, *(FString("/")));
}

void UStorage::SplitInitialActorInformation(FString in, FString alt_path, int& actorID, FString& objectName, UClass*& actorClass, UStaticMesh*& staticMesh, FVector& scale, FString& tag) {
	in.RemoveSpacesInline();			// remove Spaces
	TArray<FString> data;
	int numElements = in.ParseIntoArray(data, *(FString(",")));
	actorID = FCString::Atoi(*(data[0]));
	objectName = data[1];
	actorClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *data[2]));
	if (actorClass == nullptr) { // if the actorClass could not be loaded, attempt to load it from the alternate path 
		actorClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *getAlternatePath(data[2], alt_path)));
	}
	staticMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *data[3]));
	if (staticMesh == nullptr) { // if the staticMesh could not be loaded, attempt to load it from the alternate path 
		staticMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *getAlternatePath(data[3], alt_path)));
	}

	scale = { FCString::Atof(*(data[4])), FCString::Atof(*(data[5])), FCString::Atof(*(data[6])) };
	tag = data[7];
}

FString UStorage::Conv_GetFilePathToClass(const AActor* actor) {
	return actor->GetClass()->GetPathName();
}

void UStorage::InitCSVWrite(const FString filename, bool& success) {
	if (!Recording_Actor_Positions) {
		success = false;
		return;
	}
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*(Storage_Path))) {
		PlatformFile.CreateDirectoryTree(*(Storage_Path));
	}
	csvFileHandle = PlatformFile.OpenWrite(*(Storage_Path + filename), false);
	success = csvFileHandle != nullptr;
}

void UStorage::CSVWriteLine(FString line) {
	if (Recording_Actor_Positions) {
		int lastComma;
		line.FindLastChar(',', lastComma);
		line[lastComma] = '\n';	// The last character will always be a comma due to the automatic actor addition. Can be changed to line-break
		csvFileHandle->Write((const uint8*)TCHAR_TO_UTF8(*line), lastComma + 1);
	}
}

void UStorage::CloseCSVWrite() {
	if (csvFileHandle) {
		csvFileHandle->Flush(true);
		delete csvFileHandle;
	}
}

void UStorage::InitCSVRead(FString filename, bool& success) {
	csvFileStream = std::ifstream(TCHAR_TO_UTF8(*(Storage_Path + filename)));
	success = csvFileStream.is_open();
}
void UStorage::CloseCSVRead() {
	csvFileStream.close();
}

void UStorage::LoadNextCSVLine_backwardsCompatible(int RecordingTypeVersion, bool& successfull, float& timeStamp, TArray<FTransform>& jointPoses, TArray<float>& fingerStates, FTransform& headPose, TArray<int>& objIDs, TArray<FTransform>& objPoses) {
	fingerStates.SetNum(6); // Set size of finger states to 6: 3 x (finger + fingerTip)
	if (!RecordingTypeVersion) {
		FVector headLocation; // headPose
		fingerStates[1] = fingerStates[3] = fingerStates[5] = 0; // in old version, fingerTips did not move
		const int num_joints = 8; // num poses = 8
		jointPoses.SetNum(num_joints);
		for (int i = 0; i < num_joints; i++) jointPoses[i].SetLocation(FVector(0, 0, -100)); // Hide the rest of the Jaco below ground
		LoadNextCSVLine_v0(successfull, timeStamp, jointPoses.Last(), fingerStates[0], headLocation, objIDs, objPoses);
		fingerStates[2] = fingerStates[4] = fingerStates[0]; // copy state of single finger as only that is stored
		jointPoses.Last().ConcatenateRotation(FQuat(FRotator(-90,-90,0)));
		headPose.SetLocation(headLocation);
	}
	else if(RecordingTypeVersion == 1){
		LoadNextCSVLine_v1(successfull, timeStamp, jointPoses, fingerStates[0], fingerStates[1], headPose, objIDs, objPoses);
		// copy state of single finger as only that is stored
		fingerStates[2] = fingerStates[4] = fingerStates[0];
		fingerStates[3] = fingerStates[5] = fingerStates[1];
	}
	else {
		LoadNextCSVLine_v2(successfull, timeStamp, jointPoses, fingerStates, headPose, objIDs, objPoses);
	}
}

void UStorage::LoadNextCSVLine_v2(bool& successfull, float& timeStamp, TArray<FTransform>& jointPoses, TArray<float>& fingerStates, FTransform& headPose, TArray<int>& objIDs, TArray<FTransform>& objPoses) {
	std::string line;
	std::getline(csvFileStream, line);
	successfull = line.length() > 0;
	if (successfull) {
		FString in(line.c_str());
		in.RemoveSpacesInline();			// remove Spaces
		TArray<FString> string_data;
		int numElements = in.ParseIntoArray(string_data, *(FString(",")));
		const int num_joints = 8;
		const int num_known_floats = 1 + (num_joints * 7) + 2 + 7; // Number of known float values (1x timeStamp, 8x {7x joint Pose} + 1x fingerState +1x fingerTipState, 7x HeadPose)
		int offset = 0;
		timeStamp = FCString::Atof(*(string_data[offset++]));
		jointPoses.SetNum(num_joints);
		for (int i = 0; i < num_joints; i++) {
			jointPoses[i].SetTranslation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			jointPoses[i].SetRotation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			jointPoses[i].SetScale3D({ 1,1,1 });
		}
		for (int i = 0; i < fingerStates.Num(); i++) {
			fingerStates[i] = FCString::Atof(*(string_data[offset++]));
		}
		headPose.SetTranslation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
		headPose.SetRotation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
		headPose.SetScale3D({ 1,1,1 });

		const int num_object_fields = 8; // Number of csv fields for each object (1x ID, 7x Pose)
		int num_objects = (numElements - num_known_floats) / (num_object_fields);
		objIDs.SetNum(num_objects);
		objPoses.SetNum(num_objects);
		for (int i = 0; i < num_objects; i++) {
			objIDs[i] = FCString::Atoi(*(string_data[offset++]));
			objPoses[i].SetTranslation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			objPoses[i].SetRotation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			objPoses[i].SetScale3D({ 1,1,1 });
		}
	}
}

void UStorage::LoadNextCSVLine_v1(bool& successfull, float& timeStamp, TArray<FTransform>& jointPoses, float& fingerState, float& fingerTipState, FTransform& headPose, TArray<int>& objIDs, TArray<FTransform>& objPoses) {
	std::string line;
	std::getline(csvFileStream, line);
	successfull = line.length() > 0;
	if (successfull) {
		FString in(line.c_str());
		in.RemoveSpacesInline();			// remove Spaces
		TArray<FString> string_data;
		int numElements = in.ParseIntoArray(string_data, *(FString(",")));
		const int num_joints = 8;
		const int num_known_floats = 1+(num_joints*7)+2+7; // Number of known float values (1x timeStamp, 8x {7x joint Pose} + 1x fingerState +1x fingerTipState, 7x HeadPose)
		int offset = 0;
		timeStamp = FCString::Atof(*(string_data[offset++]));
		jointPoses.SetNum(num_joints);
		for (int i = 0; i < num_joints; i++) {
			jointPoses[i].SetTranslation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			jointPoses[i].SetRotation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			jointPoses[i].SetScale3D({ 1,1,1 });
		}

		fingerState = FCString::Atof(*(string_data[offset++]));
		fingerTipState = FCString::Atof(*(string_data[offset++]));
		headPose.SetTranslation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
		headPose.SetRotation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
		headPose.SetScale3D({ 1,1,1 });

		const int num_object_fields = 8; // Number of csv fields for each object (1x ID, 7x Pose)
		int num_objects = (numElements - num_known_floats) / (num_object_fields);
		objIDs.SetNum(num_objects);
		objPoses.SetNum(num_objects);
		for (int i = 0; i < num_objects; i++) {
			objIDs[i] = FCString::Atoi(*(string_data[offset++]));
			objPoses[i].SetTranslation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			objPoses[i].SetRotation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			objPoses[i].SetScale3D({ 1,1,1 });
		}
	}
}


void UStorage::LoadNextCSVLine_v0(bool& successfull, float& timeStamp, FTransform& gripperPose, float& fingerState, FVector& headLocation, TArray<int>& objIDs, TArray<FTransform>& objPoses) {
	std::string line;
	std::getline(csvFileStream, line);
	successfull = line.length() > 0;
	if (successfull) {
		FString in(line.c_str());
		in.RemoveSpacesInline();			// remove Spaces
		TArray<FString> string_data;
		int numElements = in.ParseIntoArray(string_data, *(FString(",")));
		const int num_known_floats = 12; // Number of known float values (1x timeStamp, 7x gripperPose + 1x fingerState, 3x HeadLocation)
		float data[num_known_floats];
		for (int i = 0; i < num_known_floats; i++) {
			data[i] = FCString::Atof(*(string_data[i]));
		}
		timeStamp = data[0];
		gripperPose.SetTranslation({ data[1], data[2], data[3] });
		gripperPose.SetRotation({ data[4], data[5], data[6], data[7] });
		fingerState = data[8];
		headLocation = { data[9], data[10], data[11] };

		const int num_object_fields = 8; // Number of csv fields for each object (1x ID, 7x Pose)
		int num_objects = (numElements - num_known_floats) / (num_object_fields);
		objIDs.SetNum(num_objects);
		objPoses.SetNum(num_objects);
		int offset = num_known_floats;
		for (int i = 0; i < num_objects; i++) {
			objIDs[i] = FCString::Atoi(*(string_data[offset++]));
			objPoses[i].SetTranslation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			objPoses[i].SetRotation({ FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])), FCString::Atof(*(string_data[offset++])) });
			objPoses[i].SetScale3D({ 1,1,1 });
		}
	}
}

FString UStorage::Conv_RenderTargetTextureToJSON(UTextureRenderTarget2D* PassedTexture, bool returnOnlyR)
{
	const int TextureLength = PassedTexture->SizeX * PassedTexture->SizeY;

	TArray<FColor> SurfData;
	FRenderTarget* RenderTarget = PassedTexture->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(SurfData);

	FString json = "[";
	if (returnOnlyR) {
		for (int i = 0; i < TextureLength; i++)
		{
			const FColor& Color = SurfData[i];
			json.AppendInt(Color.R);
			json.AppendChar(',');
		}
	}
	else {
		for (int i = 0; i < TextureLength; i++)
		{ 
			const FColor& Color = SurfData[i];
			json.AppendInt(Color.R);
			json.AppendChar(',');
			json.AppendInt(Color.G);
			json.AppendChar(',');
			json.AppendInt(Color.B);
			json.AppendChar(',');
		}
	}
	json[json.Len() - 1] = ']';
	return json;
}

void UStorage::NotifyPlayInEditorInfo(FString message) {
	FMessageLog("PIE").Info(FText::FromString(message));
	FMessageLog("PIE").Notify(FText::FromString(message), EMessageSeverity::Error, true);
}
