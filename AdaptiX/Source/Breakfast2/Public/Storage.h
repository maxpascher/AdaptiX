// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "CoreMinimal.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/DateTime.h"
#include "Storage.generated.h"

/**
 *
 */
UCLASS()
class BREAKFAST2_API UStorage : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:

public:
	/** Sets the Player state to Spectator Mode */
	UFUNCTION(BlueprintCallable, Category = "Storage")
		static void SetPlayerToSpectatorMode(APlayerController* player);

	/** Convert Render Target 2D to JSON. Partially copied from tensorflow-ue4 
	 * @return The JSON text als FString
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToJSON (Render Target 2D)", BlueprintAutocast), Category = "Adaptiv")
		static FString Conv_RenderTargetTextureToJSON(UTextureRenderTarget2D* InTexture, bool returnOnlyR);

	/** Loads information from the init File.
 	 * 
	 * @return True if successfull
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
		static bool loadInitFile();

	/** Tries to increment the iterator file. 
	 * @return True if successfull  
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
		static bool incrementIteratorFile();
	// GETTER
	/** Get the storage path as defined in the setup file */
	UFUNCTION(BlueprintCallable, Category = "Storage")
		static FString GetStoragePath();
	/** Calculate the waiting time between frames from the framerate provided in the setup file */
	UFUNCTION(BluePrintCallable, Category = "Storage")
		static float GetStorageTimerTime();
	/** Get the value of the variable Store_Images from the setup file
	 * @return True if images are to be stored.
	 */
	UFUNCTION(BluePrintCallable, Category = "Storage")
		static bool GetStorage_Activated_Images();
	/** Get the value of the variable Store_Headset_Images from the setup file
	 * @return True if headset images are to be stored.
	 */
	UFUNCTION(BluePrintPure, Category = "Storage")
		static bool GetStorage_Activated_Headset_Images();	
	/** Get the value of Record_Position_CSV from the setup file
	 * @return True if the poses are to be stored.
	 */
	UFUNCTION(BluePrintCallable, Category = "Storage")
		static bool GetImagesInHighRes();
	/**Returns the Value of PlayReplay from the setup file
	 * @return True if a replay is to be played
	 */
	UFUNCTION(BluePrintPure, Category = "Storage")
		static bool GetPlayReplay();
	/**Returns the Value of RecordingActivated from the setup file
	Returns true if data is to be stored in this run*/
	UFUNCTION(BluePrintPure, Category = "Storage")
		static bool GetRecordingActivated();
	/**Returns the Value of Recording_Start_Manually  from the setup file
	 * @return True if data storage is to be started manually this run
	 */
	UFUNCTION(BluePrintPure, Category = "Storage")
		static bool GetRecordingStartManually();
	// END Getter

	/** Creates a file at StoragePath + Path + FileName (within the local directory) and stores the SaveString.	
	 * @return The return value of FileSaveString
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
		static bool FileSaveString(FString Path, const FString FileName, const FString SaveString);

	/** Creates a Directory with the current Timestamp in the ReplayData folder if not already existing.
	 * @return True if a directory existed previously or has been created. False if none exists after the call. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
		static bool createReplayDataDirectory(const float GameTime, FString& CurrentPath);

	/** Stores the names of all first-level folders under Storage_Path in Folders. */
	UFUNCTION(BlueprintCallable, Category = "Storage")
		static void loadFolderNames(TArray<FString>& Folders);
	/** Exposes the Rotator to Quaternion function to Blueprint*/
	UFUNCTION(BlueprintPure, Category = "Felix")
		static void GetQuat(const FRotator in, FQuat& out);
	/** Converts a Quaternion to a string in the form  'X, Y, Z, W'*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (Quaternion)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Storage")
		static FString Conv_QuatToString(FQuat in);

	/** Exposes the respective FFileHelper function to Blueprint */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void LoadFileToStringArray(const FString filename, const FString path, TArray<FString>& out);

	/** Exposes the respective FFileHelper function to Blueprint */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void LoadFileToString(const FString filename, const FString path, FString& out);

	/** Splits an Actor initialisation string from the replay system to the individual elements
	 * If found, loads the Actor Class and Static Mesh based on the object path in the Content folder
	 * @param in String to be split
	 * @param alt_path An alternative path to be used if not found in the Content folder. If an object were under Content/a/b.c, also checks Content/alt_path/a/b.c
	 */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void SplitInitialActorInformation(FString in, FString alt_path, int& actorID, FString& objectName, UClass*& actorClass, UStaticMesh*& staticMesh, FVector& scale, FString& tag);
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ClassPath (Actor)"), Category = "Replay")
		static FString Conv_GetFilePathToClass(const AActor *actor);
	/** Initialises the csv writer to zero.
	 * Needs to be called before CloseCSVWrite if no csv has been created
	 */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void clearCSVWriterHandle();
	/** If recording is activated, initialises the CSVWriter
	 * @param filename The file to be written to
	 * @param success False if the initialised writer is nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void InitCSVWrite(FString filename, bool& success);
	/** Writes a single line to the csv file */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void CSVWriteLine(const FString line);
	/** Closes the csv file writer */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void CloseCSVWrite();
	/** Initialises the 
	 * @param filename The file to be read from
	 * @param success True if the reader is opened
	 */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void InitCSVRead(FString filename, bool& success);
	/** Loads the next CSV line and fills the variables with the data of a single line.
	 * @param RecordingTypeVersion 
			V0: EnableMe Student Setup (not storing recording type)
			V1: Storing recording type. Using Options arm/only wrist
			V2: Introducing 3-finger grasping and this numbering
	 * @warning objPoses Scale might not be correct, do not set it
	 */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void LoadNextCSVLine_backwardsCompatible(int RecordingTypeVersion, bool& successfull, float& timeStamp, TArray<FTransform>& jointPoses, TArray<float>& fingerStates, FTransform& headPose, TArray<int>& objIDs, TArray<FTransform>& objPoses);
	/** Loads the next CSV line and fills the variables with the data of a single line.
	 * Uses the newer approach with three finger grasping
	 * @warning objPoses Scale might not be correct, do not set it
	 */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void LoadNextCSVLine_v2(bool& successfull, float& timeStamp, TArray<FTransform>& jointPoses, TArray<float>& fingerStates, FTransform& headPose, TArray<int>& objIDs, TArray<FTransform>& objPoses);
	/** DEPRECATED: 
	 * Loads the next CSV line and fills the variables with the data of a single line.
	 * Uses the newer approach with single jaco joint poses
	 * @warning objPoses Scale might not be correct, do not set it
	 */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void LoadNextCSVLine_v1(bool& successfull, float& timeStamp, TArray<FTransform>& jointPoses, float& fingerState, float& fingerTipState, FTransform& headPose, TArray<int>& objIDs, TArray<FTransform>& objPoses);
	/** Deprecated for setup with BP_Wrist (i.e. without a Recording Type)Warning: objPoses Scale might not be correct, do not set it*/
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void LoadNextCSVLine_v0(bool& successfull, float& timeStamp, FTransform& gripperPose, float& fingerState, FVector& headLocation, TArray<int>& objIDs, TArray<FTransform>& objPoses);
	/** Closes the csv file reader */
	UFUNCTION(BlueprintCallable, Category = "Replay")
		static void CloseCSVRead();
	/** Exposes notification of play in editor infos with Blueprint */
	UFUNCTION(BlueprintCallable, Category = "Felix")
		static void NotifyPlayInEditorInfo(FString message);
};
