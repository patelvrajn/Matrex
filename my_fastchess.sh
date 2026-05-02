MAX_ROUNDS=50000
ELO_BOUND_MIN=0
ELO_BOUND_MAX=10
FALSE_POSITIVE_ERROR_RATE=0.05
FALSE_NEGATIVE_ERROR_RATE=0.05
ELO_MODEL="bayesian"
GAME_VARIANT="standard"
TIME_CONTROL="8+0.08"
TRANSPOSITION_TABLE_SIZE=128
OPENING_BOOK="UHO_Lichess_4852_v1.epd"
NUMBER_OF_THREADS=4

if [ "$1" == "FULL" ]; then
    fastchess -rounds $MAX_ROUNDS -repeat -sprt elo0=$ELO_BOUND_MIN elo1=$ELO_BOUND_MAX alpha=$FALSE_POSITIVE_ERROR_RATE beta=$FALSE_NEGATIVE_ERROR_RATE model=$ELO_MODEL -variant $GAME_VARIANT -each tc=$TIME_CONTROL -engine cmd=./matrex_altered name=altered option.Hash=$TRANSPOSITION_TABLE_SIZE -engine cmd=./matrex_base name=base option.Hash=$TRANSPOSITION_TABLE_SIZE -pgnout file=san -recover -log file=log level=trace realtime=true engine=true -openings file=$OPENING_BOOK format=epd order=random -concurrency $NUMBER_OF_THREADS
elif [ "$1" == "NO_LOG" ]; then
    fastchess -rounds $MAX_ROUNDS -repeat -sprt elo0=$ELO_BOUND_MIN elo1=$ELO_BOUND_MAX alpha=$FALSE_POSITIVE_ERROR_RATE beta=$FALSE_NEGATIVE_ERROR_RATE model=$ELO_MODEL -variant $GAME_VARIANT -each tc=$TIME_CONTROL -engine cmd=./matrex_altered name=altered option.Hash=$TRANSPOSITION_TABLE_SIZE -engine cmd=./matrex_base name=base option.Hash=$TRANSPOSITION_TABLE_SIZE -pgnout file=san -recover -openings file=$OPENING_BOOK format=epd order=random -concurrency $NUMBER_OF_THREADS
else
    echo "Error: Invalid argument!" >&2; 
    exit 1;
fi
