import Text.Pandoc
import Data.Char (toUpper)

main :: IO ()
main = toJsonFilter capitalizeHeaders

capitalizeHeaders :: Block -> Block
capitalizeHeaders (Header 1 attr xs) = Header 1 attr $ bottomUp capitalize xs
capitalizeHeaders x = x

capitalize :: Inline -> Inline
capitalize (Str xs) = Str $ map toUpper xs
capitalize x = x
