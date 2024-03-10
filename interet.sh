#/usr/bin/env bash

_interet_module()
{
	local cur prev OPTS MODE_OPT
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"

	MODE_OPT="jour quinzaine jourbourso"

	case $prev in
		'-h'|'--help')
			return 0
			;;
		'-o'|'--operation'|'-t'|'--taux')
			local realcur
			realcur="${cur##*,}"
			prefix="${cur%$realcur}"
			COMPREPLY=( $(compgen -f -P "$prefix" -- $realcur) )
			compopt -o filenames
			return 0
			;;
		'-d'|'--date')
			local realcur
			realcur="${cur##*,}"
			prefix="${cur%$realcur}"
			COMPREPLY=( $(compgen -W $(date +'%F') -- $realcur))
			return 0
			;;
		'-m'|'--mode')
			local realcur
			realcur="${cur##*,}"
			prefix="${cur%$realcur}"
			COMPREPLY=( $(compgen -W "jour quinzaine jourbourso" -- $realcur))
			return 0
			;;
	esac
	case $cur in
		*)
			OPTS="--operation
				--taux
				--date
				--mode
				--help"
			COMPREPLY=( $(compgen -W "${OPTS[*]}" -- $cur) )
			return 0
			;;
	esac
}
complete -F _interet_module interet
