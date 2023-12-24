#!/usr/bin/env stack
-- stack --install-ghc runghc --package turtle
{-# LANGUAGE AllowAmbiguousTypes #-}
{-# LANGUAGE FlexibleContexts    #-}
{-# LANGUAGE OverloadedStrings   #-}

{-
To compile:
  stack ghc -- -O2 -dynamic -no-keep-hi-files -no-keep-o-files -Wall tagger.hs -o ~/.local/bin/tagger
-}

import           Control.Applicative
import           Data.Either
import           Data.List
import           Data.List.Split
import           Data.Functor
import           Data.Maybe
import qualified Data.Text           as T
import           Options.Applicative
import qualified Text.Parsec         as P
import qualified Turtle
import qualified Data.Text.IO        as TIO
import System.Directory

-- Arguments -----------------------------------------------------------

data Arguments = Arguments
  { _find     :: Maybe String
  , _useFzf   :: Maybe String
  , _tags     :: Bool
  , _untagged :: Bool }

argParser :: Parser Arguments
argParser =
  Arguments <$>
  optional
    (Options.Applicative.strOption $
     long "find" <>
     short 'f' <>
     metavar "\"tag1 tag2 tagN\"" <>
     help "Search entries by tags.") <*>
  optional
    (Options.Applicative.strOption $
     long "find-fullnames" <>
     short 'x' <> metavar "\"tag1 tag2 tagN\"" <> help "fzf") <*>
  Options.Applicative.switch
    (long "list-tags" <> short 'l' <> help "List existing tags.") <*>
  Options.Applicative.switch
    (long "list-not-tagged" <> short 'n' <> help "List files without tag.")

opts :: ParserInfo Arguments
opts = info (argParser <**> helper)
            (fullDesc <> progDesc "Search tags" <> Options.Applicative.header "Tagger")

readArgs :: Arguments -> IO ()
readArgs (Arguments tags Nothing False False) = do
  printFilesNamesFull tags
readArgs (Arguments Nothing tags False False) = do
  fzf tags
readArgs (Arguments Nothing Nothing True False) =
  (nub . sort <$> listTags) >>= \lst -> mapM_ putStrLn lst
readArgs (Arguments Nothing Nothing False True) =
  splitCmd listNotTagged >>= \lst -> mapM_ TIO.putStrLn lst
readArgs _ = return ()

-- Arguments END -----------------------------------------------------------

filesDirectory :: T.Text
filesDirectory = "/Your/Root/Path/Here"

listTaggedFiles :: T.Text
listTaggedFiles = "find " <> filesDirectory <> " -name '*' -type f -not -path '*/\\.git/*' -not -path '*/ltximg/*' -print | grep '\\+..*+@'"

listNotTagged :: T.Text
listNotTagged = "find " <> filesDirectory <> " -name '*' -type f -not -path '*/\\.git/*' -not -path '*/ltximg/*' -printf '%f\\n' | grep -v '^+..*+@'"

-- |Run a command and split results into a list,
-- |then remove the last element (empty line).
splitCmd :: Turtle.MonadIO m => Turtle.Text -> m [Turtle.Text]
splitCmd commandStr = Turtle.strict (Turtle.inshell commandStr empty) >>=
                           \shResult -> pure (T.splitOn "\n" shResult) >>=
                           \splitResult -> return $ init splitResult -- also works: Data.List.delete "" splitResult

runParser :: String -> Either P.ParseError [String]
runParser = P.parse parseLine "(unknown)"

parseLine :: P.Parsec String () [String]
parseLine = do
  pathStr <- P.many $ P.noneOf "+"
  _ <- P.char '+'
  result <- parseCells
  _ <- P.char '+'
  _ <- P.char '@'
  fileName <- P.many1 P.anyChar
  P.eof
  return ([pathStr] ++ result ++ [fileName])

-- |Everything before , or ) is a cell.
-- |If a , is encountered, parse a new cell, otherwise return [].
parseCells :: P.Parsec String () [String]
parseCells = do
  first <- P.many $ P.noneOf ",+"
  next <- (P.char ',' >> parseCells) <|> return []
  return (first : next)

-- |Parse the files and return the parsing result with the complete path.
parseTagged :: IO [Either P.ParseError [String]]
parseTagged = splitCmd listTaggedFiles >>=
  \listOfFiles -> return $ map (runParser . T.unpack) listOfFiles

-- |Drop the first and last elements of a list.
dropFstLst :: [a] -> [a]
dropFstLst = init . tail

-- |Return a list of all tags.
listTags :: IO [String]
listTags = rights <$> parseTagged >>= \ll -> return $ concatMap dropFstLst ll

searchTags :: [String] -> IO [[String]]
searchTags tagsList =
  rights <$> parseTagged Data.Functor.<&> filter (and <$> mapM elem tagsList)

constructFileName :: Monad m => [String] -> m String
constructFileName fileList = return $
  head fileList ++ "+" ++ intercalate "," (dropFstLst fileList) ++ "+@" ++ last fileList

constructFileName' :: [String] -> [Char]
constructFileName' fileList =  head fileList ++ "+" ++ intercalate "," (dropFstLst fileList) ++ "+@" ++ last fileList

printFilesNamesFull :: Maybe String -> IO ()
printFilesNamesFull a = do
  search <- searchTags splitArgs
  assemblePath <- mapM constructFileName search
  mapM_ putStrLn assemblePath
  where
    splitArgs = Data.List.Split.splitOn " " (fromJust a)

fzf :: Maybe String -> IO ()
fzf a = do
  let splittedArgs = Data.List.Split.splitOn " " (fromJust a)
  found <- searchTags splittedArgs
  assembledPath <- mapM constructFileName found
  let t = intercalate "\n" assembledPath
  let commandStr = "echo " <> "\"" <> T.pack t <> "\"" <> " | eval fzf | xargs -0 -I {} echo {} | xargs -0 xdg-open"
  _ <- Turtle.shell commandStr empty
  return ()

appendMiddle :: a -> [a] -> [a]
appendMiddle s (x:xs) = x:s:xs

main :: IO ()
main = execParser opts >>= readArgs
