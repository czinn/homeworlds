import argparse
import io
import subprocess
import sys

def run_process(args, stdin):
  p = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
  result = p.communicate(input=stdin.encode("utf-8"))[0]
  return result.decode("utf-8")

# returns move, new_game, winner
def make_move(args, game):
  move = run_process(args, game + "\n\n")
  judge_result = run_process(["./judge"], game + "\n\n" + move)
  judge_result_split = judge_result.strip().split("\n\n")
  new_game = judge_result_split[0].strip()
  winner = int(judge_result_split[1].strip()) if len(judge_result_split) > 1 else None
  return move, new_game, winner

if __name__ == "__main__":
  parser = argparse.ArgumentParser(prog="play_game")
  parser.add_argument("initial_game", type=argparse.FileType("r"),
      help="Initial game state")

  options = parser.parse_args(sys.argv[1:])

  init_game = options.initial_game.read().strip()
  options.initial_game.close()

  # run game to completion!

  game, winner = init_game, None
  while winner is None:
    move, game, winner = make_move(["./main"], game)

    print(move.strip())
    print("---")
    print(game.strip())
    print("---")

  print("Player {} wins!".format(winner))
