import sys
import sc2reader

def main():
    path = sys.argv[1]
    replay = sc2reader.load_replay(path)
    playerOneName = replay.players[0].name
    playerTwoName = replay.players[1].name
    playerOneUrl = replay.players[0].url
    playerTwoUrl = replay.players[1].url
    PlayerOneResult = replay.players[0].result
    PlayerTwoResult = replay.players[1].result
    replayType = replay.type
    mapName = replay.map_name
    endTime = replay.end_time
    length = replay.length
    replayFilename = replay.filename

    workFilename = replayFilename + "_output.txt"
    f =open(workFilename,'w')
    f.write("Player One (name,url,result): ")
    f.write(playerOneName)
    f.write(',')
    f.write(playerOneUrl)
    f.write(',')
    f.write(PlayerOneResult)
    f.write("\nPlayer Two (name,url,result): ")
    f.write(playerTwoName)
    f.write(',')
    f.write(playerTwoUrl)
    f.write(',')
    f.write(PlayerTwoResult)
    f.write("\nReplay Info (type,map,endtime,length): ")
    f.write(replayType)
    f.write(',')
    f.write(mapName)
    f.write(';')
    f.write(str(endTime))
    f.write(',')
    f.write(str(length))
    f.write('\n')

    f.close()

if __name__ == '__main__':
    main()
