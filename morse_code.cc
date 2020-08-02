#include <iostream>
#include <numeric>

namespace morse
{
    template<std::size_t N>
    struct FixedString
    {
        wchar_t buf[N + 1]{};

        constexpr FixedString() = default;

        constexpr FixedString(wchar_t const *s)
        {
            std::copy(s, s + N, buf);
        }

        template<std::size_t S>
        constexpr FixedString(FixedString<S> const &other)
        {
            std::copy(other.buf, other.buf + std::min(S, N), buf);
        }

        auto constexpr operator==(FixedString const &other) const
        {
            return std::equal(buf, buf + N, other.buf);
        }

        auto constexpr operator[](std::size_t index) const
        { return buf[index]; }

        auto constexpr &operator[](std::size_t index)
        { return buf[index]; }

        std::size_t constexpr size() const
        { return N; }

        friend auto &operator<<(std::wostream &stream, const FixedString &string)
        {
            stream << string.buf;
            return stream;
        }

        auto constexpr begin() { return buf; }

        auto constexpr end() { return buf + N;}

        const auto constexpr begin() const { return buf; }

        const auto constexpr end() const { return buf + N;}
    };
    template<std::size_t N>
    FixedString(wchar_t const (&)[N]) -> FixedString<N - 1>;

    /* a linear binary tree of morse code */
    FixedString static constexpr decode_map { L"<ETIANMSURWDKGOHVFÜL<PJBXCYZQÖ<54Ŝ3É<<2<È+<<<<16=/<<<<<7<<<8<90<<<<<<<<<<<<?_<<<<\"<<.<<<<@<<<'<<-<<<<<<<<;!<()<<<<<,<<<<:<<<<<<<" };

    template<FixedString string>
    auto constexpr encode()
    {
        /* std::toupper is not constexpr */
        auto constexpr to_upper = [](auto const& ch) { return (L'a' < ch && ch < L'z') ?  ch - (L'a' - L'A') : ch; };

        auto constexpr find_letter_index = [](auto const& ch) {
            /* if char is a ' ' use the index for '_' */
            return std::distance(decode_map.buf, std::find(decode_map.buf, decode_map.buf + decode_map.size(), ch));
        };

        auto constexpr letter_length = [](auto const &ch) {
            std::size_t letter_index = std::distance(decode_map.buf, std::find(decode_map.buf, decode_map.buf + decode_map.size(), ch));
            std::size_t result = 0;
            for(;letter_index; --letter_index /= 2) result++;
            return result;
        };

        std::size_t constexpr result_size = std::accumulate(string.buf, string.buf + string.size(), std::size_t {0},
                [&](std::size_t total_size, auto const& ch) { return total_size + letter_length(to_upper(ch)) + 1; })  - !!string.size();

        FixedString<result_size> result{};
        std::size_t write_index = 0;
        std::size_t old_write_index = 0;
        std::size_t current_index = 0;

        for(const auto& ch : string)
        {
            current_index = find_letter_index(to_upper(ch));

            old_write_index = write_index;
            for(;current_index; current_index /= 2)
                result[write_index++] = L"-."[current_index--%2];
            /* reverse */
            std::reverse(result.buf + old_write_index, result.buf + write_index);
            /* add a ' ' if we have enough space left */
            if(write_index < result.size()) result[write_index++] = ' ';
        }
        return result;
    }

    template<FixedString string>
    auto constexpr decode()
    {
        auto constexpr find_token_count = [] {
            std::size_t result = 0;
            for(std::size_t i = 0; i < string.size(); ++i)
            {
                result += i + 1 >= string.size() || (string[i] + 1 >> 1 == 23 && string[i + 1] == L' ');
            }
            return result;
        };
        std::size_t constexpr result_size = find_token_count();
        FixedString<result_size> result{};
        /* find first char that is not a space */
        std::size_t read_index = std::distance(string.begin(), std::find_if(string.begin(), string.end(),
                [](auto const& ch) { return ch != L' '; }));

        std::size_t current_index = 0;
        for(auto& ch : result)
        {
            current_index = 1;
            for (;read_index < string.size() && string[read_index] != L' '; ++read_index)
                current_index = current_index * 2 + (string[read_index] == L'-');
            /* go over ' ' */
            ++read_index;
            /* if char was not found replace it with a space */
            ch = current_index > decode_map.size() ? L' ' : decode_map[current_index - 1];
        }
        return result;
    }

}

int main()
{
      constexpr morse::FixedString input_text { L"ABCDEFGHIJKLMNOPQRSTUVWSYZ1234567890 ?"};
      auto constexpr encoded = morse::encode<input_text>();
      auto constexpr decoded = morse::decode<encoded>();
      static_assert(decoded == input_text);
      std::wcout << decoded << L'\n';
}
