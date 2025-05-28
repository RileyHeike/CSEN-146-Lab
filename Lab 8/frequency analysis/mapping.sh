ciphertext_file="ciphertext"

english_freq="ETAOINSHRDLUCMWFGYPBVKJXQZ"

# Count letter frequencies and print
freqs=$(cat "$ciphertext_file" | tr -cd 'A-Z' | fold -w1 | sort | uniq -c | sort -nr)

echo "Ciphertext letter frequencies:"
echo "$freqs"
echo

# Extract letters into string sorted by freq
cipher_letters=$(echo "$freqs" | awk '{print $2}' | tr -d '\n')
echo "Cipher letters by frequency: $cipher_letters"

# Create map of cipher letters to English letters by freq
mapping=""
len=${#cipher_letters}
for ((i=0; i < len; i++)); do
	c=${cipher_letters:$i:1}
	e=${english_freq:$i:1}
	mapping+="$c:$e "
done

echo "Initial substitution mapping (cipher:plain):"
echo "$mapping"
echo
